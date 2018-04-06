#include "AddCustomNodeOperation.h"
#include <utility>
#include "../node/CustomNode.h"

using namespace AxiomModel;

AddCustomNodeOperation::AddCustomNodeOperation(AxiomModel::Project *project, AxiomModel::SurfaceRef surfaceRef, QString name, QPoint pos)
    : HistoryOperation(true), project(project), surfaceRef(std::move(surfaceRef)), name(std::move(name)), pos(pos) {

}

void AddCustomNodeOperation::forward() {
    auto surface = project->findSurface(surfaceRef);
    assert(surface);

    insertIndex = surface->items().size();

    auto defaultSize = QSize(3, 2);
    surface->addItem(std::make_unique<CustomNode>(surface, surface->items().size(), name, pos, defaultSize));
}

void AddCustomNodeOperation::backward() {
    auto surface = project->findSurface(surfaceRef);
    assert(surface);

    surface->items()[insertIndex]->remove();
}
