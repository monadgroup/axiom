#pragma once

#include "CollectionViewOperators.h"

namespace AxiomModel {

    template<class TO, class TI>
    TO find(const CollectionView<TI> &input, const QUuid &uuid) {
        for (const auto &item : input) {
            if (item->uuid() == uuid) {
                return dynamic_cast<TO>(item);
            }
        }
        return nullptr;
    }

    template<class TI>
    TI find(const CollectionView<TI> &input, const QUuid &uuid) {
        return find<TI, TI>(input, uuid);
    }

    template<class TO, class TI>
    Promise<TO> findLater(const CollectionView<TI> &input, QUuid uuid) {
        return getFirst(derive<TO, TI>(input, [uuid](const TI &base) -> std::optional<TO> {
            if (base->uuid() == uuid) {
                auto cast = dynamic_cast<TO>(base);
                return cast ? cast : std::optional<TO>();
            }
            return std::optional<TO>();
        }));
    }

    template<class TI>
    CollectionView<TI> filterChildren(const CollectionView<TI> &input, QUuid parentUuid) {
        return derive<TI, TI>(input, [parentUuid](const TI &base) -> std::optional<TI> {
            return base->parentUuid() == parentUuid ? base : std::optional<TI>();
        });
    }

}
