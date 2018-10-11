#pragma once

#include <iterator>
#include <optional>
#include <tuple>
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

    template<class Output, class Sequence, class Functor>
    class FilterMapGenerator {
    public:
        using Item = Output;
        using Manager = GeneratorManager<FilterMapGenerator, Sequence, Functor>;

        Sequence sequence;
        typename Sequence::iterator sequenceBegin;
        typename Sequence::iterator sequenceEnd;
        Functor functor;

        FilterMapGenerator(Sequence seq, Functor functor)
            : sequence(std::move(seq)), sequenceBegin(sequence.begin()), sequenceEnd(sequence.end()),
              functor(std::move(functor)) {}

        std::optional<Item> next() {
            while (sequenceBegin != sequenceEnd) {
                auto mappedValue = functor(std::move(*sequenceBegin));
                *sequenceBegin++;

                if (mappedValue) {
                    return mappedValue;
                }
            }

            return std::nullopt;
        }
    };

    template<class Sequence, class Functor>
    class FilterGenerator {
    public:
        using Item = typename Sequence::value_type;
        using Manager = GeneratorManager<FilterGenerator, Sequence, Functor>;

        Sequence sequence;
        typename Sequence::iterator sequenceBegin;
        typename Sequence::iterator sequenceEnd;
        Functor functor;

        explicit FilterGenerator(Sequence seq, Functor functor)
            : sequence(std::move(seq)), sequenceBegin(sequence.begin()), sequenceEnd(sequence.end()),
              functor(std::move(functor)) {}

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

    template<class Output, class Sequence, class Functor>
    class MapGenerator {
    public:
        using Item = Output;
        using Manager = GeneratorManager<MapGenerator, Sequence, Functor>;

        Sequence sequence;
        typename Sequence::iterator sequenceBegin;
        typename Sequence::iterator sequenceEnd;
        Functor functor;

        explicit MapGenerator(Sequence seq, Functor functor)
            : sequence(std::move(seq)), sequenceBegin(sequence.begin()), sequenceEnd(sequence.end()),
              functor(std::move(functor)) {}

        std::optional<Item> next() {
            if (sequenceBegin != sequenceEnd) {
                auto mappedValue = functor(std::move(*sequenceBegin));
                *sequenceBegin++;

                return mappedValue;
            }
            return std::nullopt;
        }
    };

    template<class Sequence>
    class FlattenGenerator {
    public:
        using Item = typename Sequence::value_type::value_type;
        using Manager = GeneratorManager<FlattenGenerator, Sequence>;

        Sequence sequence;
        typename Sequence::iterator sequenceBegin;
        typename Sequence::iterator sequenceEnd;
        std::optional<typename Sequence::value_type> innerSequence;
        std::optional<typename Sequence::value_type::iterator> innerBegin;
        std::optional<typename Sequence::value_type::iterator> innerEnd;

        explicit FlattenGenerator(Sequence seq)
            : sequence(std::move(seq)), sequenceBegin(sequence.begin()), sequenceEnd(sequence.end()) {}

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

        explicit IterGenerator(Collection *collection)
            : sequenceBegin(collection->begin()), sequenceEnd(collection->end()) {}

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

    template<class Generator>
    class Sequence {
    public:
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

        // functions to create chained sequences
        template<class MapOutput, class FilterMapFunctor>
        Sequence<FilterMapGenerator<MapOutput, Sequence, FilterMapFunctor>> filterMap(FilterMapFunctor functor) {
            return Sequence(GeneratorManager(*this, functor));
        }

        template<class FilterFunctor>
        Sequence<FilterGenerator<Sequence, FilterFunctor>> filter(FilterFunctor functor) {
            return Sequence(GeneratorManager(*this, functor));
        }

        template<class MapOutput, class MapFunctor>
        Sequence<MapGenerator<MapOutput, Sequence, MapFunctor>> map(MapFunctor functor) {
            return Sequence(GeneratorManager(*this, functor));
        }

        Sequence<FlattenGenerator<Sequence>> flatten() { return Sequence(GeneratorManager(*this)); }

        template<class Output>
        using MapPtr = Output (*)(Item);

        template<class MapOutput>
        Sequence<MapGenerator<MapOutput, Sequence, MapPtr<MapOutput>>> dynamicCast() {
            return map(&Sequence::dynamicCast<MapOutput>);
        }

        template<class MapOutput>
        Sequence<MapGenerator<MapOutput, Sequence, MapPtr<MapOutput>>> staticCast() {
            return map(&Sequence::staticCast<MapOutput>);
        }

        template<class MapOutput>
        Sequence<MapGenerator<MapOutput, Sequence, MapPtr<MapOutput>>> reinterpretCast() {
            return map(&Sequence::reinterpretCast<MapOutput>);
        }

    private:
        template<class Output>
        static Output dynamicCast(Item input) {
            return dynamic_cast<Output>(input);
        }

        template<class Output>
        static Output staticCast(Item input) {
            return static_cast<Output>(input);
        }

        template<class Output>
        static Output reinterpretCast(Item input) {
            return reinterpret_cast<Output>(input);
        }
    };

    // functions to create new iterators
    template<class Item>
    Sequence<BlankGenerator<Item>> blank() {
        return Sequence(GeneratorManager<BlankGenerator<Item>>());
    }

    template<class Item>
    Sequence<OnceGenerator<Item>> once(Item item) {
        return Sequence(GeneratorManager(std::move(item)));
    }

    template<class Collection>
    Sequence<IterGenerator<Collection>> iter(Collection *collection) {
        return Sequence(GeneratorManager(collection));
    }

    template<class Collection>
    Sequence<IntoIterGenerator<Collection>> intoIter(Collection collection) {
        return Sequence(GeneratorManager(std::forward(collection)));
    }
}
