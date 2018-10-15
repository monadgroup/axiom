#pragma once

#include <QtCore/QHash>
#include <QtCore/QUuid>
#include <optional>

namespace AxiomModel {

    template<class S>
    class IndexedSequence {
    public:
        using Sequence = S;
        using Item = typename Sequence::value_type;

        using value_type = Item;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using iterator = typename Sequence::iterator;

        Sequence sequence;

        IndexedSequence(Sequence seq, const QHash<QUuid, Item> *index) : sequence(std::move(seq)), index(index) {}

        iterator begin() { return sequence.begin(); }

        iterator end() { return sequence.end(); }

        bool empty() { return sequence.empty(); }

        size_t size() { return sequence.size(); }

        std::optional<Item> find(const QUuid &id) {
            auto iter = index->find(id);
            if (iter == index->end())
                return std::nullopt;
            else
                return iter.value();
        }

    private:
        const QHash<QUuid, Item> *index;
    };

    template<class Sequence>
    IndexedSequence<Sequence> indexSequence(Sequence sequence,
                                            const QHash<QUuid, typename Sequence::value_type> *index) {
        return IndexedSequence<Sequence>(std::move(sequence), index);
    }
}
