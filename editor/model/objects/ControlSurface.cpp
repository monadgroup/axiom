#include "ControlSurface.h"

#include "Control.h"
#include "Node.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"

using namespace AxiomModel;

ControlSurface::ControlSurface(const QUuid &uuid, const QUuid &parentUuid, AxiomModel::ModelRoot *root)
    : ModelObject(ModelType::CONTROL_SURFACE, uuid, parentUuid, root), _node(find(root->nodes(), parentUuid)),
      _controls(findChildrenWatch(root->controls(), uuid)), _grid(staticCastWatch<GridItem *>(_controls)) {
}

std::unique_ptr<ControlSurface> ControlSurface::create(const QUuid &uuid, const QUuid &parentUuid,
                                                       AxiomModel::ModelRoot *root) {
    return std::make_unique<ControlSurface>(uuid, parentUuid, root);
}

std::unique_ptr<ControlSurface> ControlSurface::deserialize(QDataStream &stream, const QUuid &uuid,
                                                            const QUuid &parentUuid, AxiomModel::ModelRoot *root) {
    return create(uuid, parentUuid, root);
}

void ControlSurface::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    ModelObject::serialize(stream, parent, withContext);
}

void ControlSurface::remove() {
    while (!_controls.empty()) {
        (*_controls.begin())->remove();
    }
    ModelObject::remove();
}
