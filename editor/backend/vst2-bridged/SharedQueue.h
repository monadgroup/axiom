#pragma once

#include <optional>
#include <atomic>
#include <array>
#include <string>
#include <cassert>
#include <QtCore/QtGlobal>

#ifdef Q_OS_WIN
#include "WinQueueSync.h"
#elif defined(Q_OS_UNIX)
#include "UnixQueueSync.h"
#endif

namespace AxiomBackend {
    class Phantom {
    public:
        Phantom() = delete;
        Phantom(const Phantom &) = delete;
        Phantom(Phantom &&) = delete;
        Phantom &operator=(const Phantom &) = delete;
        Phantom &operator=(Phantom &&) = delete;
        ~Phantom() = delete;
    };

    // Disables "auto functions" like move, copy, destructors, etc since the value inside might not be initialized.
    template<class Item>
    class DisableAutos {
    public:
        union {
            Item maybeItem;

            // todo: is this needed?
            Phantom phantom;
        };

        DisableAutos() {}
        ~DisableAutos() {}
    };

    // The buffer used for a queue - maintains a fixed-capacity storage and a read/write index. Items within the
    // read/write indices are assumed to be initialized, and move constructors/operators and destructors will only be
    // called on them.
    template<class Item, size_t Capacity>
    class QueueBuffer {
    public:
        std::atomic_size_t readIndex;
        std::atomic_size_t writeIndex;
        std::array<DisableAutos<Item>, Capacity> storage;

        QueueBuffer() : readIndex(0), writeIndex(0) {}

        QueueBuffer(const QueueBuffer &) = delete;

        QueueBuffer(QueueBuffer &&other) noexcept : readIndex(0), writeIndex(0) {
            auto currentReadIndex = other.readIndex.exchange(0);
            auto currentWriteIndex = other.writeIndex.exchange(0);

            if (currentReadIndex > currentWriteIndex) {
                callMoveConstructors(currentReadIndex, Capacity);
                callMoveConstructors(0, currentWriteIndex);
            } else {
                callMoveConstructors(currentReadIndex, currentWriteIndex, other);
            }

            readIndex.store(currentReadIndex);
            writeIndex.store(currentWriteIndex);
        }

        QueueBuffer &operator=(const QueueBuffer &) = delete;

        QueueBuffer &operator=(QueueBuffer &&other) noexcept {
            ~QueueBuffer();
            ::new (this) QueueBuffer(other);
            return *this;
        }

        ~QueueBuffer() {
            auto currentReadIndex = readIndex.exchange(0);
            auto currentWriteIndex = writeIndex.exchange(0);

            if (currentReadIndex > currentWriteIndex) {
                callDestructors(currentReadIndex, Capacity);
                callDestructors(0, currentWriteIndex);
            } else {
                callDestructors(currentReadIndex, currentWriteIndex);
            }
        }

    private:
        void callMoveConstructors(size_t beginIndex, size_t endIndex, QueueBuffer &&other) {
            for (auto i = beginIndex; i < endIndex; i++) {
                ::new (&storage[i].maybeItem) Item(std::move(other.storage[i].maybeItem));
            }
        }

        void callDestructors(size_t beginIndex, size_t endIndex) {
            for (auto i = beginIndex; i < endIndex; i++) {
                storage[i].maybeItem.~Item();
            }
        }
    };

    template<class Item, size_t Capacity>
    class Queue {
    public:
        using value_type = Item;

        class SeparateData {
        public:
            QueueSync::SeparateData consumeData;
            QueueSync::SeparateData produceData;

            explicit SeparateData(const std::string &id)
                : consumeData(id + ".consume"),
                  produceData(id + ".produce") {}
        };

        size_t size() const {
            auto currentReadIndex = container.readIndex.load();
            auto currentWriteIndex = container.writeIndex.load();

            if (currentReadIndex > currentWriteIndex) {
                return capacity() - currentReadIndex + currentWriteIndex;
            } else {
                return currentWriteIndex - currentReadIndex;
            }
        }

        size_t capacity() const {
            return Capacity;
        }

        bool empty() const {
            return container.readIndex.load() == container.writeIndex.load();
        }

        bool full() const {
            return countToIndex(container.writeIndex.load() + 1) == container.readIndex.load();
        }

        // Tries to push the item to the queue, returning the item if no space is available.
        std::optional<Item> tryPush(Item value, SeparateData &sep) {
            produceSync.lock(sep.produceData);

            auto currentWriteIndex = container.writeIndex.load();
            auto nextWriteIndex = countToIndex(currentWriteIndex + 1);
            if (nextWriteIndex == container.readIndex.load()) {
                // The queue is full
                produceSync.unlock(sep.produceData);
                return value;
            }

            ::new (&container.storage[currentWriteIndex].maybeItem) Item(std::move(value));

            container.writeIndex.store(nextWriteIndex);

            produceSync.unlock(sep.produceData);
            produceSync.wakeOne(sep.produceData);
            return std::nullopt;
        }

        // Tries to push the item to the queue, aborting if no space is available.
        void push(Item value, SeparateData &sep) {
            if (tryPush(std::move(value), sep)) {
                assert(false && "Cannot push item, queue is full");
                abort();
            }
        }

        // Pushes an item to the queue, blocking the current thread if no space is available
        void pushWhenAvailable(Item value, SeparateData &sep) {
            consumeSync.lock(sep.consumeData);

            while (auto returnedItem = tryPush(std::move(value), sep)) {
                value = std::move(*returnedItem);

                // Suspend until space is available to push into
                consumeSync.wait(sep.consumeData);
            }

            consumeSync.unlock(sep.consumeData);
        }

        // Tries to pop an item off the queue, returning it if available.
        std::optional<Item> tryPop(SeparateData &sep) {
            consumeSync.lock(sep.consumeData);

            auto currentReadIndex = container.readIndex.load();
            if (currentReadIndex == container.writeIndex.load()) {
                // The queue is empty
                consumeSync.unlock(sep.consumeData);
                return std::nullopt;
            }

            auto &itm = container.storage[currentReadIndex].maybeItem;
            auto readValue = std::move(itm);
            itm.~Item();
            container.readIndex.store(countToIndex(currentReadIndex + 1));

            consumeSync.unlock(sep.consumeData);
            consumeSync.wakeOne(sep.consumeData);

            return readValue;
        }

        Item pop(SeparateData &sep) {
            if (auto val = tryPop(sep)) {
                return *val;
            }
            assert(false && "Cannot pop item, queue is empty");
            abort();
        }

        Item popWhenAvailable(SeparateData &sep) {
            produceSync.lock(sep.produceData);

            while (true) {
                if (auto poppedItem = tryPop(sep)) {
                    produceSync.unlock(sep.produceData);
                    return *poppedItem;
                }

                produceSync.wait(sep.produceData);
            }
        }

    private:
        QueueSync consumeSync;
        QueueSync produceSync;
        QueueBuffer<Item, Capacity> container;

        size_t countToIndex(size_t count) {
            return count % capacity();
        }
    };

}
