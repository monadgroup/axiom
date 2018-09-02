#include "ControlSurface.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "Control.h"
#include "Node.h"

using namespace AxiomModel;

ControlSurface::ControlSurface(const QUuid &uuid, const QUuid &parentUuid, AxiomModel::ModelRoot *root)
    : ModelObject(ModelType::CONTROL_SURFACE, uuid, parentUuid, root), _node(find(root->nodes(), parentUuid)),
      _controls(findChildrenWatch(root->controls(), uuid)),
      _grid(staticCastWatch<GridItem *>(_controls), QPoint(0, 0)) {
    _node->sizeChanged.connect(this, &ControlSurface::setSize);
    _node->deselected.connect(&_grid, &GridSurface::deselectAll);
    _grid.hasSelectionChanged.connect(this, [this](bool hasSelection) {
                                          if (hasSelection) _node->select(true);
                                      });
    setSize(_node->size());
}

std::unique_ptr<ControlSurface> ControlSurface::create(const QUuid &uuid, const QUuid &parentUuid,
                                                       AxiomModel::ModelRoot *root) {
    return std::make_unique<ControlSurface>(uuid, parentUuid, root);
}

void ControlSurface::remove() {
    while (!_controls.empty()) {
        (*_controls.begin())->remove();
    }
    ModelObject::remove();
}

void ControlSurface::setSize(QSize size) {
    _grid.grid().maxRect = nodeToControl(QPoint(size.width(), size.height()));
}
