#include "ModulePreviewCanvas.h"

#include "editor/model/schematic/Schematic.h"
#include "../node/NodeItem.h"
#include "../connection/WireItem.h"

using namespace AxiomGui;
using namespace AxiomModel;

ModulePreviewCanvas::ModulePreviewCanvas(const Schematic *schematic) {
    for (const auto &item : schematic->items()) {
        if (auto node = dynamic_cast<Node *>(item.get())) {
            addNode(node);
        }
    }

    for (const auto &wire : schematic->wires()) {
        addWire(wire.get());
    }
}

void ModulePreviewCanvas::addNode(AxiomModel::Node *node) {
    auto item = new NodeItem(node, nullptr);
    item->setZValue(1);
    addItem(item);
}

void ModulePreviewCanvas::addWire(AxiomModel::ConnectionWire *wire) {
    auto item = new WireItem(this, wire);
    item->setZValue(0);
    addItem(item);
}
