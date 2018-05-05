#include "ModelObject.h"

#include "ModelRoot.h"
#include "objects/NodeSurface.h"
#include "objects/CustomNode.h"
#include "objects/ControlSurface.h"
#include "objects/Control.h"
#include "objects/Connection.h"
#include "../util.h"

using namespace AxiomModel;

ModelObject::ModelObject(ModelType modelType, const QUuid &uuid, const QUuid &parentUuid, ModelRoot *root)
    : PoolObject(uuid, parentUuid, root->pool()), _modelType(modelType), _root(root) {
}

std::unique_ptr<ModelObject> ModelObject::deserialize(QDataStream &stream, const QUuid &parent, ModelRoot *root) {
    uint8_t typeInt; stream >> typeInt;
    QUuid uuid; stream >> uuid;
    QUuid parentUuid; stream >> parentUuid;

    if (parentUuid.isNull()) parentUuid = parent;

    auto type = (ModelType) typeInt;
    switch (type) {
        case ModelType::NODE_SURFACE: return NodeSurface::deserialize(stream, uuid, parentUuid, root);
        case ModelType::NODE: return Node::deserialize(stream, uuid, parentUuid, root);
        case ModelType::CONTROL_SURFACE: return ControlSurface::deserialize(stream, uuid, parentUuid, root);
        case ModelType::CONTROL: return Control::deserialize(stream, uuid, parentUuid, root);
        case ModelType::CONNECTION: return Connection::deserialize(stream, uuid, parentUuid, root);
        case ModelType::LIBRARY_ENTRY:break;
        case ModelType::HISTORY_ACTION:break;
    }

    unreachable;
}

void ModelObject::serialize(QDataStream &stream) const {
    stream << (uint8_t) modelType();
    stream << uuid();
    stream << parentUuid();
}

void ModelObject::remove() {
    removed.trigger();
    cleanup.trigger();
    PoolObject::remove();
}
