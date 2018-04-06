#include "AddNodeOperation.h"

#include "../node/CustomNode.h"
#include "../node/GroupNode.h"

using namespace AxiomModel;

AddNodeOperation::AddNodeOperation(AxiomModel::Project *project, AxiomModel::SurfaceRef surfaceRef, Node::Type type, QString name, QPoint pos)
    : HistoryOperation(true), project(project), surfaceRef(std::move(surfaceRef)), type(type), name(std::move(name)), pos(pos) {

}

void AddNodeOperation::forward() {
    auto surface = project->findSurface(surfaceRef);
    assert(surface);

    insertIndex = surface->items().size();
    auto defaultSize = QSize(3, 2);

    switch (type) {
        case Node::Type::CUSTOM:
            surface->addItem(std::make_unique<CustomNode>(surface, surface->items().size(), name, pos, defaultSize));
            break;
        case Node::Type::GROUP:
            surface->addItem(std::make_unique<GroupNode>(surface, surface->items().size(), name, pos, defaultSize));
            break;
        case Node::Type::IO:break;
    }
}

void AddNodeOperation::backward() {
    auto surface = project->findSurface(surfaceRef);
    assert(surface);

    surface->items()[insertIndex]->remove();
}
