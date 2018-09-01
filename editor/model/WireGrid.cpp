#include "WireGrid.h"

#include <algorithm>

using namespace AxiomModel;

void WireGrid::addPoint(QPoint point, AxiomModel::ConnectionWire *wire) {
    auto index = cells.find(point);
    if (index == cells.end()) {
        cells.emplace(point, {wire});
    } else {
        index->second.push_back(wire);
    }
}

void WireGrid::removePoint(QPoint point, AxiomModel::ConnectionWire *wire) {
    auto index = cells.find(point);
    if (index == cells.end()) return;

    index->second.erase(std::remove(index->second.begin(), index->second.end(), wire), index->second.end());
    if (index->second.empty()) {
        cells.erase(point);
    }
}

void WireGrid::addLine(QPoint start, QPoint end, AxiomModel::ConnectionWire *wire) {}
