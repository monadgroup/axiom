#include "ModulePreviewCanvas.h"

#include "editor/model/schematic/Schematic.h"
#include "../node/NodeItem.h"
#include "../connection/WireItem.h"

using namespace AxiomGui;
using namespace AxiomModel;

ModulePreviewCanvas::ModulePreviewCanvas(const Schematic *schematic) {
    // todo: this could be refactored with SchematicCanvas
    for (const auto &item : schematic->items()) {
        if (auto node = dynamic_cast<Node *>(item.get())) {
            addNode(node);
        }
    }

    for (const auto &wire : schematic->wires()) {
        addWire(wire.get());
    }

    // connect to model
    connect(schematic, &Schematic::itemAdded,
            [this](AxiomModel::GridItem *item) {
                if (auto node = dynamic_cast<Node *>(item)) {
                    addNode(node);
                }
            });
    connect(schematic, &Schematic::itemAdded,
            this, &ModulePreviewCanvas::contentChanged);
    connect(schematic, &Schematic::wireAdded,
            this, &ModulePreviewCanvas::addWire);
}

void ModulePreviewCanvas::addNode(AxiomModel::Node *node) {
    connect(node, &Node::posChanged,
            this, &ModulePreviewCanvas::contentChanged);
    connect(node, &Node::sizeChanged,
            this, &ModulePreviewCanvas::contentChanged);
    connect(node, &Node::removed,
            this, &ModulePreviewCanvas::contentChanged);

    auto item = new NodeItem(node, nullptr);
    item->setZValue(1);
    addItem(item);
}

void ModulePreviewCanvas::addWire(AxiomModel::ConnectionWire *wire) {
    auto item = new WireItem(this, wire);
    item->setZValue(0);
    addItem(item);
}
