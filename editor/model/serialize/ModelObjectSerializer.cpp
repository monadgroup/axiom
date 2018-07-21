#include "ModelObjectSerializer.h"

#include "../IdentityReferenceMapper.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/Connection.h"
#include "../objects/Control.h"
#include "../objects/ControlSurface.h"
#include "../objects/Node.h"
#include "../objects/NodeSurface.h"
#include "ConnectionSerializer.h"
#include "ControlSerializer.h"
#include "ControlSurfaceSerializer.h"
#include "HistorySerializer.h"
#include "NodeSerializer.h"
#include "NodeSurfaceSerializer.h"

using namespace AxiomModel;

void ModelObjectSerializer::serializeRoot(AxiomModel::ModelRoot *root, QDataStream &stream) {
    serializeChunk(stream, QUuid(), dynamicCast<ModelObject *>(root->pool().sequence()));
    HistorySerializer::serialize(root->history(), stream);
}

std::vector<ModelObject *> ModelObjectSerializer::deserializeChunk(QDataStream &stream, uint32_t version,
                                                                   ModelRoot *root, const QUuid &parent,
                                                                   AxiomModel::ReferenceMapper *ref) {
    std::vector<ModelObject *> usedObjects;

    uint32_t objectCount;
    stream >> objectCount;
    usedObjects.reserve(objectCount);
    for (uint32_t i = 0; i < objectCount; i++) {
        std::cout << "Deserializing model object at " << stream.device()->pos() << std::endl;
        QByteArray objectBuffer;
        stream >> objectBuffer;
        QDataStream objectStream(&objectBuffer, QIODevice::ReadOnly);

        auto newObject = deserialize(objectStream, version, root, parent, ref);
        usedObjects.push_back(newObject.get());
        root->pool().registerObj(std::move(newObject));
    }

    return std::move(usedObjects);
}

std::unique_ptr<ModelRoot> ModelObjectSerializer::deserializeRoot(QDataStream &stream, uint32_t version,
                                                                  Project *project) {
    auto modelRoot = std::make_unique<ModelRoot>(project);
    IdentityReferenceMapper ref;
    deserializeChunk(stream, version, modelRoot.get(), QUuid(), &ref);
    modelRoot->history() =
        HistorySerializer::deserialize(stream, version, modelRoot.get(), std::move(modelRoot->history().applyer()));
    return std::move(modelRoot);
}

void ModelObjectSerializer::serialize(AxiomModel::ModelObject *obj, QDataStream &stream, const QUuid &parent) {
    stream << obj->uuid();
    stream << (parent == obj->parentUuid() ? QUuid() : obj->parentUuid());
    stream << (uint8_t) obj->modelType();
    serializeInner(obj, stream);
}

std::unique_ptr<ModelObject> ModelObjectSerializer::deserialize(QDataStream &stream, uint32_t version,
                                                                AxiomModel::ModelRoot *root, const QUuid &parent,
                                                                AxiomModel::ReferenceMapper *ref) {
    QUuid uuid;
    stream >> uuid;
    uuid = ref->mapUuid(uuid);

    QUuid parentUuid;
    stream >> parentUuid;
    if (parentUuid.isNull())
        parentUuid = parent;
    else
        parentUuid = ref->mapUuid(parentUuid);

    uint8_t typeInt;
    stream >> typeInt;

    std::cout << "Deserializing " << (uint64_t) typeInt << " UUID = " << uuid.toString().toStdString()
              << " (parent = " << parentUuid.toString().toStdString() << ")" << std::endl;
    return deserializeInner(stream, version, root, (ModelObject::ModelType) typeInt, uuid, parentUuid, ref);
}

void ModelObjectSerializer::serializeInner(AxiomModel::ModelObject *obj, QDataStream &stream) {
    if (auto nodeSurface = dynamic_cast<NodeSurface *>(obj))
        return NodeSurfaceSerializer::serialize(nodeSurface, stream);
    if (auto node = dynamic_cast<Node *>(obj)) return NodeSerializer::serialize(node, stream);
    if (auto controlSurface = dynamic_cast<ControlSurface *>(obj))
        return ControlSurfaceSerializer::serialize(controlSurface, stream);
    if (auto control = dynamic_cast<Control *>(obj)) return ControlSerializer::serialize(control, stream);
    if (auto connection = dynamic_cast<Connection *>(obj)) return ConnectionSerializer::serialize(connection, stream);
    unreachable;
}

std::unique_ptr<ModelObject> ModelObjectSerializer::deserializeInner(QDataStream &stream, uint32_t version,
                                                                     AxiomModel::ModelRoot *root,
                                                                     AxiomModel::ModelObject::ModelType type,
                                                                     const QUuid &uuid, const QUuid &parent,
                                                                     AxiomModel::ReferenceMapper *ref) {
    switch (type) {
    case ModelObject::ModelType::NODE_SURFACE:
        return NodeSurfaceSerializer::deserialize(stream, version, uuid, parent, ref, root);
    case ModelObject::ModelType::NODE:
        return NodeSerializer::deserialize(stream, version, uuid, parent, ref, root);
    case ModelObject::ModelType::CONTROL_SURFACE:
        return ControlSurfaceSerializer::deserialize(stream, version, uuid, parent, ref, root);
    case ModelObject::ModelType::CONTROL:
        return ControlSerializer::deserialize(stream, version, uuid, parent, ref, root);
    case ModelObject::ModelType::CONNECTION:
        return ConnectionSerializer::deserialize(stream, version, uuid, parent, ref, root);
    default:
        unreachable;
    }
}
