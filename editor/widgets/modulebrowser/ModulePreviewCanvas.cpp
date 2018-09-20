#include "ModulePreviewCanvas.h"

#include "../connection/WireItem.h"
#include "../node/NodeItem.h"
#include "editor/model/objects/Connection.h"
#include "editor/model/objects/Node.h"
#include "editor/model/objects/NodeSurface.h"

using namespace AxiomGui;
using namespace AxiomModel;

ModulePreviewCanvas::ModulePreviewCanvas(NodeSurface *surface) {
    // create items for all nodes and wires that already exist
    // todo: this could be refactored with NodeSurfaceCanvas
    for (const auto &node : surface->nodes()) {
        addNode(node);
    }

    for (const auto &connection : surface->connections()) {
        connection->wire().then(this, [this](std::unique_ptr<ConnectionWire> &wire) { addWire(wire.get()); });
    }

    // connect to model
    surface->nodes().itemAdded.connect(this, &ModulePreviewCanvas::addNode);
    surface->connections().itemAdded.connect(this, [this](Connection *connection) {
        connection->wire().then(this, [this](std::unique_ptr<ConnectionWire> &wire) { addWire(wire.get()); });
    });
}

void ModulePreviewCanvas::addNode(AxiomModel::Node *node) {
    node->posChanged.connect(this, &ModulePreviewCanvas::contentChanged);
    node->sizeChanged.connect(this, &ModulePreviewCanvas::contentChanged);
    node->removed.connect(this, &ModulePreviewCanvas::contentChanged);

    auto item = new NodeItem(node, nullptr);
    item->setZValue(1);
    addItem(item);
}

void ModulePreviewCanvas::addWire(AxiomModel::ConnectionWire *wire) {
    auto item = new WireItem(this, wire);
    item->setZValue(0);
    addItem(item);
}
