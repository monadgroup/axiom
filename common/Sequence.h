#pragma once

#include <iterator>
#include <optional>
#include <tuple>
#include <type_traits>
#include <vector>

namespace AxiomCommon {

    template<class Generator>
    class SequenceIterator {
    public:
        using Item = typename Generator::Item;
        using self_type = SequenceIterator;
        using iterator_category = std::forward_iterator_tag;

        SequenceIterator() = default;

        explicit SequenceIterator(Generator generator) : generator(std::move(generator)) { increment(); }

        bool ended() const { return !generator; }

        self_type operator++() {
            self_type i = *this;
            increment();
            return i;
        }

        const self_type operator++(int) {
            increment();
            return *this;
        }

        bool operator==(const SequenceIterator &rhs) const {
            // if either iterator has ended, iterators are only equal if both have ended
            if (ended() || rhs.ended()) {
                return ended() && rhs.ended();
            }

            // otherwise, just compare indexes
            return _index == rhs._index;
        }

        bool operator!=(const SequenceIterator &rhs) const { return !(*this == rhs); }

        Generator &operator*() { return *currentValue; }

        Generator *operator->() { return &*currentValue; }

    private:
        size_t _index = 0;
        std::optional<Generator> generator;
        std::optional<Item> currentValue;

        void increment() {
            if (generator) {
                currentValue = generator->next();
                _index++;

                if (!currentValue) generator.reset();
            }
        }
    };

    template<class G, class... Args>
    class GeneratorManager {
    public:
        using Generator = G;
        using Sequence = typename Generator::Sequence;

        std::tuple<Args...> args;

        explicit GeneratorManager(Args... args) : args(std::make_tuple(std::forward(args)...)) {}

        SequenceIterator<Generator> begin() const {
            return createIteratorIndexed(std::make_index_sequence<sizeof...(Args)>());
        }

    private:
        template<size_t... I>
        SequenceIterator<Generator> createIteratorIndexed(std::index_sequence<I...>) {
            return SequenceIterator(Generator(std::get<I>(args)...));
        }
    };

    template<class Output>
    class BlankGenerator {
    public:
        using Item = Output;
        using Manager = GeneratorManager<BlankGenerator>;

        BlankGenerator() = default;

        std::optional<Item> next() { return std::nullopt; }
    };

    template<class Output>
    class OnceGenerator {
    public:
        using Item = Output;
        using Manager = GeneratorManager<OnceGenerator>;

        std::optional<Item> item;

        explicit OnceGenerator(Item item) : item(std::move(item)) {}

        std::optional<Item> next() { return std::move(item); }
    };

    template<class S, class Functor>
    class FilterMapGenerator {
    public:
        using Sequence = S;
        using Input = typename Sequence::value_type;
        using Item = typename std::result_of<Functor(Input)>::type;
        using Manager = GeneratorManager<FilterMapGenerator, Sequence &, Functor &>;

        static constexpr size_t SequenceIndex = 1;
        static constexpr size_t FunctorIndex = 2;

        typename Sequence::iterator sequenceBegin;
        typename Sequence::iterator sequenceEnd;
        Functor &functor;

        FilterMapGenerator(Sequence &sequence, Functor &functor)
            : sequenceBegin(sequence.begin()), sequenceEnd(sequence.end()), functor(functor) {}

        static std::optional<Item> mapFilter(Manager &manager, Input input) {
            return std::get<FunctorIndex>(manager.args)(std::forward(input));
        }

        std::optional<Item> next() {
            while (sequenceBegin != sequenceEnd) {
                auto mappedValue = mapFilter(functor, std::move(*sequenceBegin));
                *sequenceBegin++;

                if (mappedValue) {
                    return mappedValue;
                }
            }

            return std::nullopt;
        }
    };

    template<class S, class Functor>
    class FilterGenerator {
    public:
        using Sequence = S;
        using Item = typename Sequence::value_type;
        using Input = Item;
        using Manager = GeneratorManager<FilterGenerator, Sequence &, Functor &>;

        static constexpr size_t SequenceIndex = 1;
        static constexpr size_t FunctorIndex = 2;

        typename Sequence::iterator sequenceBegin;
        typename Sequence::iterator sequenceEnd;
        Functor &functor;

        explicit FilterGenerator(Sequence &sequence, Functor &functor)
            : sequenceBegin(sequence.begin()), sequenceEnd(sequence.end()), functor(functor) {}

        static std::optional<Item> mapFilter(Manager &manager, Input input) {
            if (std::get<FunctorIndex>(manager.args)(input))
                return input;
            else
                return std::nullopt;
        }

        std::optional<Item> next() {
            while (sequenceBegin != sequenceEnd) {
                auto nextValue = std::move(*sequenceBegin);
                sequenceBegin++;

                if (functor(nextValue)) {
                    return nextValue;
                }
            }

            return std::nullopt;
        }
    };

    template<class S, class Functor>
    class MapGenerator {
    public:
        using Sequence = S;
        using Input = typename Sequence::value_type;
        using Item = typename std::result_of<Functor(Input)>::type;
        using Manager = GeneratorManager<MapGenerator, Sequence &, Functor &>;

        static constexpr size_t SequenceIndex = 1;
        static constexpr size_t FunctorIndex = 2;

        typename Sequence::iterator sequenceBegin;
        typename Sequence::iterator sequenceEnd;
        Functor &functor;

        explicit MapGenerator(Sequence &sequence, Functor &functor)
            : sequenceBegin(sequence.begin()), sequenceEnd(sequence.end()), functor(functor) {}

        static std::optional<Item> mapFilter(Manager &manager, Input input) {
            return std::get<FunctorIndex>(manager)(std::move(input));
        }

        std::optional<Item> next() {
            if (sequenceBegin != sequenceEnd) {
                auto mappedValue = functor(std::move(*sequenceBegin));
                *sequenceBegin++;

                return mappedValue;
            }
            return std::nullopt;
        }
    };

    template<class S, class Data, class Functor>
    class FunctorGenerator {};

    template<class S>
    class FlattenGenerator {
    public:
        using Sequence = S;
        using Item = typename Sequence::value_type::value_type;
        using Manager = GeneratorManager<FlattenGenerator, Sequence &>;

        typename Sequence::iterator sequenceBegin;
        typename Sequence::iterator sequenceEnd;
        std::optional<typename Sequence::value_type> innerSequence;
        std::optional<typename Sequence::value_type::iterator> innerBegin;
        std::optional<typename Sequence::value_type::iterator> innerEnd;

        explicit FlattenGenerator(Sequence &sequence) : sequenceBegin(sequence.begin()), sequenceEnd(sequence.end()) {}

        std::optional<Item> next() {
            while (!innerBegin || !innerEnd || *innerBegin == *innerEnd) {
                if (*sequenceBegin == *sequenceEnd) {
                    return std::nullopt;
                }

                innerSequence = std::move(*sequenceBegin);
                innerBegin = innerSequence->begin();
                innerEnd = innerSequence->end();
                sequenceBegin++;
            }

            auto innerValue = std::move(*innerBegin);
            innerBegin++;
            return innerValue;
        }
    };

    template<class Collection>
    class IterGenerator {
    public:
        using Item = typename Collection::value_type *;
        using Manager = GeneratorManager<IterGenerator, Collection *>;

        typename Collection::iterator sequenceBegin;
        typename Collection::iterator sequenceEnd;

        explicit IterGenerator(Collection &collection)
            : sequenceBegin(collection.begin()), sequenceEnd(collection.end()) {}

        std::optional<Item> next() {
            if (sequenceBegin != sequenceEnd) {
                auto nextValue = &*sequenceBegin;
                *sequenceBegin++;

                return nextValue;
            }
            return std::nullopt;
        }
    };

    template<class Collection>
    class IntoIterGenerator {
    public:
        using Item = typename Collection::value_type;
        using Manager = GeneratorManager<IntoIterGenerator, Collection>;

        Collection ownedCollection;
        typename Collection::iterator sequenceBegin;
        typename Collection::iterator sequenceEnd;

        explicit IntoIterGenerator(Collection collection)
            : ownedCollection(std::move(collection)), sequenceBegin(ownedCollection.begin()),
              sequenceEnd(ownedCollection.end()) {}

        std::optional<Item> next() {
            if (sequenceBegin != sequenceEnd) {
                auto nextValue = std::move(*sequenceBegin);
                *sequenceBegin++;

                return nextValue;
            }
            return std::nullopt;
        }
    };

    template<class G>
    class Sequence {
    public:
        using Generator = G;
        using Manager = typename Generator::Manager;
        using Item = typename Generator::Item;

        using value_type = Item;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using iterator = SequenceIterator<Generator>;

        Manager manager;

        explicit Sequence(Manager manager) : manager(std::move(manager)) {}

        iterator begin() const { return manager.begin(); }

        iterator end() const { return manager.end(); }

        bool empty() const { return begin() == end(); }

        size_t size() const {
            size_t acc = 0;
            for (auto i = begin(); i != end(); i++) {
                acc++;
            }
            return acc;
        }

        std::vector<Item> collect() {
            std::vector<Item> result;
            for (auto &itm : *this) {
                result.push_back(std::move(itm));
            }
            return result;
        }

        std::optional<Item> takeAt(size_t index) {
            size_t currentIndex = 0;
            for (auto &itm : *this) {
                if (currentIndex == index) {
                    return std::move(itm);
                }
                currentIndex++;
            }
            return std::nullopt;
        }
    };

    template<class Item>
    class IteratorAdapter {
        virtual std::unique_ptr<IteratorAdapter> clone() const = 0;
        virtual bool ended() const = 0;
        virtual void increment() = 0;
        virtual bool equals(IteratorAdapter *adapter) const = 0;
        virtual Item *deref() const = 0;
    };

    template<class Item, class IterType>
    class SpecifiedIteratorAdapter : public IteratorAdapter<Item> {
    public:
        IterType iterator;

        explicit SpecifiedIteratorAdapter(IterType iterator) : iterator(std::move(iterator)) {}

        std::unique_ptr<IteratorAdapter<Item>> clone() const override {
            return std::make_unique<SpecifiedIteratorAdapter>(iterator);
        }

        bool ended() const override { return iterator.ended(); }

        void increment() override { ++iterator; }

        bool equals(IteratorAdapter<Item> *adapter) const override {
            // review: does this need to be done safely?
            auto specifiedAdapter = (SpecifiedIteratorAdapter *) adapter;
            return iterator == specifiedAdapter->iterator;
        }

        Item *deref() const override { return &*iterator; }
    };

    template<class Item>
    class IteratorAdapterIterator {
    public:
        using self_type = IteratorAdapterIterator;
        using iterator_category = std::forward_iterator_tag;

        explicit IteratorAdapterIterator(std::unique_ptr<IteratorAdapter<Item>> adapter)
            : adapter(std::move(adapter)) {}

        IteratorAdapterIterator(const IteratorAdapterIterator &a) : adapter(a.adapter->clone()) {}

        IteratorAdapterIterator(IteratorAdapterIterator &&) noexcept = default;

        IteratorAdapterIterator &operator=(const IteratorAdapterIterator &a) {
            adapter = a.adapter->clone();
            return *this;
        }

        IteratorAdapterIterator &operator=(IteratorAdapterIterator &&a) noexcept = default;

        self_type operator++() {
            self_type i = *this;
            adapter->increment();
            return i;
        }

        const self_type operator++(int) {
            adapter->increment();
            return *this;
        }

        bool operator==(const IteratorAdapterIterator &rhs) const { return adapter->equals(rhs.adapter.get()); }

        bool operator!=(const IteratorAdapterIterator &rhs) const { return !(*this == rhs); }

        Item &operator*() { return *adapter->deref(); }

        Item *operator->() { return adapter->deref(); }

    private:
        std::unique_ptr<IteratorAdapter<Item>> adapter;
    };

    // boxes a sequence so we don't need to know where the values come from
    template<class I>
    class BoxedSequence {
        class BoxAdapter {
        public:
            virtual std::unique_ptr<IteratorAdapter<I>> begin() const = 0;
            virtual std::unique_ptr<IteratorAdapter<I>> end() const = 0;
            virtual bool empty() const = 0;
            virtual size_t size() const = 0;
        };

        template<class Sequence>
        class SpecifiedBoxAdapter : public BoxAdapter {
        public:
            Sequence sequence;

            explicit SpecifiedBoxAdapter(Sequence sequence) : sequence(std::move(sequence)) {}

            std::unique_ptr<IteratorAdapter<I>> begin() const override {
                return std::make_unique<SpecifiedIteratorAdapter>(sequence.begin());
            }

            std::unique_ptr<IteratorAdapter<I>> end() const override {
                return std::make_unique<SpecifiedIteratorAdapter>(sequence.end());
            }

            bool empty() const override { return sequence.empty(); }

            size_t size() const override { return sequence.size(); }
        };

    public:
        using Item = I;

        using value_type = Item;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using iterator = IteratorAdapterIterator<Item>;

        template<class Sequence>
        explicit BoxedSequence(Sequence sequence)
            : adapter(std::make_unique<SpecifiedBoxAdapter<Sequence>>(std::move(sequence))) {}

        iterator begin() const { return IteratorAdapterIterator(adapter->begin()); }

        iterator end() const { return IteratorAdapterIterator(adapter->end()); }

        bool empty() const { return adapter->empty(); }

        size_t size() const { return adapter->size(); }

    private:
        std::unique_ptr<BoxAdapter> adapter;
    };
}
