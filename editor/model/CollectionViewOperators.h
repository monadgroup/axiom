#pragma once

#include "CollectionView.h"

namespace AxiomModel {

    template<class TO, class TI>
    CollectionView<TO> derive(const CollectionView<TI> &input, std::function<std::optional<TO>(const TI &)> func) {
        return CollectionView<TO>(input, func);
    }

    template<class TO, class TI>
    CollectionView<TO> deriveFunc(const CollectionView<TI> &input, std::optional<TO> (*func)(const TI &)) {
        return derive(input, std::function(func));
    };

    template<class TO, class TI>
    CollectionView<TO> filterType(const CollectionView<TI> &input) {
        return derive<TO, TI>(input, [](const TI &base) -> std::optional<TO> {
            auto convert = dynamic_cast<TO>(base);
            if (!convert) return std::optional<TO>();
            else return convert;
        });
    };

    template<class TO, class TI>
    CollectionView<TO> staticCast(const CollectionView<TI> &input) {
        return derive<TO, TI>(input, [](const TI &base) -> std::optional<TO> {
            return static_cast<TO>(base);
        });
    };

    template<class TO, class TI>
    CollectionView<TO> reinterpetCast(const CollectionView<TI> &input) {
        return derive<TO>(input, [](const TI &base) -> std::optional<TO> {
            return reinterpret_cast<TO>(base);
        });
    };

    template<class TI>
    Promise<TI> getFirst(CollectionView<TI> input) {
        Promise<TI> result;

        input.itemAdded.listen([result](const TI &item) mutable {
            result.resolve(item);
        });

        return std::move(result);
    }

    template<class TI>
    void collect(const CollectionView<TI> &input, const std::vector<TI> &result) {
        for (const auto &itm : input) {
            result.push_back(itm);
        }
    }

    template<class TI>
    std::vector<TI> collect(const CollectionView<TI> &input) {
        std::vector<TI> result;
        collect(input, result);
        return result;
    }

}
