#include "ControlSurface.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "Control.h"
#include "Node.h"

using namespace AxiomModel;

ControlSurface::ControlSurface(const QUuid &uuid, const QUuid &parentUuid, AxiomModel::ModelRoot *root)
    : ModelObject(ModelType::CONTROL_SURFACE, uuid, parentUuid, root),
      _node(find(root->nodes().sequence(), parentUuid)),
      _controls(AxiomCommon::boxWatchSequence(findChildrenWatch(root->controls(), uuid))),
      _grid(AxiomCommon::boxWatchSequence(
                AxiomCommon::staticCastWatch<GridItem *>(AxiomCommon::refWatchSequence(&_controls))),
            false, QPoint(0, 0)) {
    _node->sizeChanged.connect(this, &ControlSurface::setSize);
    _node->deselected.connect(&_grid, &GridSurface::deselectAll);
    _grid.hasSelectionChanged.connect(this, [this](bool hasSelection) {
        if (hasSelection) _node->select(true);
    });
    _grid.gridChanged.connect(this, &ControlSurface::updateControlsOnTopRow);
    setSize(_node->size());

    _grid.tryFlush();
    updateControlsOnTopRow();
}

std::unique_ptr<ControlSurface> ControlSurface::create(const QUuid &uuid, const QUuid &parentUuid,
                                                       AxiomModel::ModelRoot *root) {
    return std::make_unique<ControlSurface>(uuid, parentUuid, root);
}

void ControlSurface::remove() {
    while (!_controls.sequence().empty()) {
        (*_controls.sequence().begin())->remove();
    }
    ModelObject::remove();
}

void ControlSurface::setSize(QSize size) {
    _grid.grid().maxRect = nodeToControl(QPoint(size.width(), size.height()));
}

void ControlSurface::updateControlsOnTopRow() {
    auto hasControlsOnTopRow = false;
    for (auto i = 0; i < _grid.grid().maxRect.x(); i++) {
        if (_grid.grid().getCell(QPoint(i, 0))) {
            hasControlsOnTopRow = true;
            break;
        }
    }

    if (hasControlsOnTopRow != _controlsOnTopRow) {
        _controlsOnTopRow = hasControlsOnTopRow;
        controlsOnTopRowChanged(hasControlsOnTopRow);
    }
}
