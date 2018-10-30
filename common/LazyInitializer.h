#pragma once

#include <optional>

namespace AxiomCommon {

    template<class Data>
    class LazyInitializer {
    public:
        class Ref {
        public:
            Data &operator*() { return *initializer->data; }
            Data *operator->() { return &*initializer->data; }

            explicit Ref(LazyInitializer *initializer) : initializer(initializer) {}

            ~Ref() { initializer->free(); }

            LazyInitializer *initializer;
        };

        LazyInitializer() = default;

        Ref get() {
            useCount++;
            if (!data) {
                data.emplace();
            }

            return Ref(this);
        }

    private:
        size_t useCount = 0;
        std::optional<Data> data;

        void free() {
            assert(useCount > 0);
            useCount--;
            if (useCount == 0) {
                data.reset();
            }
        }
    };
}
