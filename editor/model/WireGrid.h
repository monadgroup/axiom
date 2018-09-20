#pragma once

#include <QtCore/QHash>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <vector>

#include "common/Event.h"
#include "common/Hookable.h"

inline uint qHash(const QPoint &);

namespace AxiomModel {

    class ConnectionWire;

    struct LineIndex {
        size_t count;
        size_t index;
    };

    class WireGrid : public AxiomCommon::Hookable {
    public:
        enum class Direction { HORIZONTAL, VERTICAL };

        AxiomCommon::Event<> gridChanged;

        void addPoint(QPoint point, ConnectionWire *wire, Direction direction);

        void removePoint(QPoint point, ConnectionWire *wire);

        void addRegion(QRect region, ConnectionWire *wire);

        void removeRegion(QRect region, ConnectionWire *wire);

        LineIndex getRegionIndex(QRect region, ConnectionWire *wire);

        void tryFlush();

    private:
        struct CellLists {
            std::vector<ConnectionWire *> horizontal;
            std::vector<ConnectionWire *> vertical;
        };

        QHash<QPoint, CellLists> cells;

        bool _isDirty = false;
    };
}
