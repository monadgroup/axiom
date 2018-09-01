#pragma once

#include <QtCore/QPoint>
#include <unordered_map>
#include <vector>

namespace AxiomModel {

    class ConnectionWire;

    struct LineIndex {
        size_t count;
        size_t index;
    };

    class WireGrid {
    public:
        void addPoint(QPoint point, ConnectionWire *wire);

        void removePoint(QPoint point, ConnectionWire *wire);

        void addLine(QPoint start, QPoint end, ConnectionWire *wire);

        void removeLine(QPoint start, QPoint end, ConnectionWire *wire);

        LineIndex getLineIndex(QPoint start, QPoint end, ConnectionWire *wire);

    private:
        std::unordered_map<QPoint, std::vector<ConnectionWire *>> cells;
    };
}
