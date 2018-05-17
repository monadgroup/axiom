#pragma once

#include "Sequence.h"

namespace AxiomModel {

    template<class OutputItem, class InputItem>
    class SequenceMapFilter {
    public:
        using sequence_type = Sequence<OutputItem>;
        using value_type = typename sequence_type::value_type;
        using reference = typename sequence_type::reference;
        using const_reference = typename sequence_type::const_reference;
        using pointer = typename sequence_type::pointer;
        using const_pointer = typename sequence_type::const_pointer;
        using iterator = typename sequence_type::iterator;
        using const_iterator = typename sequence_type::const_iterator;
        using next_functor = std::function<std::optional<OutputItem>(const InputItem &)>;

        template<class InputCollection>
        SequenceMapFilter(InputCollection input, next_functor next)
            : _next(next), _sequence([next, input]() {
            auto begin = input.begin();
            auto end = input.end();
            return [next, begin, end]() mutable -> std::optional<OutputItem> {
                while (begin != end) {
                    auto newVal = next(*begin);
                    begin++;
                    if (newVal) return *newVal;
                }
                return std::optional<OutputItem>();
            };
        }) {}

        const next_functor &next() const { return _next; }

        sequence_type &sequence() { return _sequence; }

        const sequence_type &sequence() const { return _sequence; }

        const_iterator begin() const {
            return _sequence.begin();
        }

        const_iterator end() const {
            return _sequence.end();
        }

        bool empty() const {
            return _sequence.empty();
        }

        size_t size() const {
            return _sequence.size();
        }

    private:
        next_functor _next;
        sequence_type _sequence;
    };

}
