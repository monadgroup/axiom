#pragma once

#include <optional>

namespace AxiomCommon {

    template<class Item, class InternalIter>
    class SingleIter {
    public:
        SingleIter(InternalIter begin, InternalIter end) : begin(std::move(begin)), end(std::move(end)) {}

        std::optional<Item *> next() {
            if (begin == end) return std::nullopt;
            if (!isFirstIter) {
                ++begin;
                if (begin == end) return std::nullopt;
            } else
                isFirstIter = false;

            return &*begin;
        }

    private:
        bool isFirstIter = true;
        InternalIter begin;
        InternalIter end;
    };
}
