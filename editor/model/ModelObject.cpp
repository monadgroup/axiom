#include "ModelObject.h"

#include "ModelRoot.h"
#include "SequenceOperators.h"
#include "ReferenceMapper.h"
#include "objects/NodeSurface.h"
#include "objects/CustomNode.h"
#include "objects/ControlSurface.h"
#include "objects/Connection.h"
#include "../util.h"

using namespace AxiomModel;

ModelObject::ModelObject(ModelType modelType, const QUuid &uuid, const QUuid &parentUuid, ModelRoot *root)
    : PoolObject(uuid, parentUuid, &root->pool()), _modelType(modelType), _root(root) {
}

std::unique_ptr<ModelObject> ModelObject::deserialize(QDataStream &stream, const QUuid &parent, ReferenceMapper *ref,
                                                      ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    uuid = ref->mapUuid(uuid);

    QUuid parentUuid;
    stream >> parentUuid;
    if (parentUuid.isNull()) parentUuid = parent;
    else parentUuid = ref->mapUuid(parentUuid);

    return deserialize(stream, uuid, parentUuid, ref, root);
}

std::unique_ptr<ModelObject> ModelObject::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                      ReferenceMapper *ref, AxiomModel::ModelRoot *root) {
    uint8_t typeInt;
    stream >> typeInt;
    switch ((ModelType) typeInt) {
        case ModelType::NODE_SURFACE:
            return NodeSurface::deserialize(stream, uuid, parentUuid, ref, root);
        case ModelType::NODE:
            return Node::deserialize(stream, uuid, parentUuid, ref, root);
        case ModelType::CONTROL_SURFACE:
            return ControlSurface::deserialize(stream, uuid, parentUuid, ref, root);
        case ModelType::CONTROL:
            return Control::deserialize(stream, uuid, parentUuid, ref, root);
        case ModelType::CONNECTION:
            return Connection::deserialize(stream, uuid, parentUuid, ref, root);
    }

    unreachable;
}

void ModelObject::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    if (withContext) {
        stream << uuid();
        stream << (parent == parentUuid() ? QUuid() : parentUuid());
    }

    stream << (uint8_t) modelType();
}

Sequence<ModelObject *> ModelObject::links() {
    return blank<ModelObject *>();
}

void ModelObject::remove() {
    removed.trigger();
    PoolObject::remove();
}
