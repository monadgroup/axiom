#include "ModelRoot.h"

#include "ModelObject.h"
#include "WatchSequenceOperators.h"
#include "IdentityReferenceMapper.h"
#include "objects/NodeSurface.h"
#include "objects/Node.h"
#include "objects/ControlSurface.h"
#include "objects/Connection.h"

using namespace AxiomModel;

ModelRoot::ModelRoot(Project *project) : _project(project),
                                         _nodeSurfaces(dynamicCastWatch<NodeSurface *>(_pool.sequence())),
                                         _nodes(dynamicCastWatch<Node *>(_pool.sequence())),
                                         _controlSurfaces(dynamicCastWatch<ControlSurface *>(_pool.sequence())),
                                         _controls(dynamicCastWatch<Control *>(_pool.sequence())),
                                         _connections(dynamicCastWatch<Connection *>(_pool.sequence())) {}

ModelRoot::ModelRoot(Project *project, QDataStream &stream) : ModelRoot(project) {
    IdentityReferenceMapper ref;
    deserializeChunk(stream, QUuid(), &ref);
    _history = HistoryList(stream, this);
}

void ModelRoot::serialize(QDataStream &stream) {
    serializeChunk(stream, QUuid(), dynamicCast<ModelObject *>(_pool.sequence()));
    _history.serialize(stream);
}

std::vector<ModelObject *> ModelRoot::deserializeChunk(QDataStream &stream, const QUuid &parent, ReferenceMapper *ref) {
    std::vector<ModelObject *> usedObjects;

    uint32_t objectCount;
    stream >> objectCount;
    usedObjects.reserve(objectCount);
    for (uint32_t i = 0; i < objectCount; i++) {
        QByteArray objectBuffer;
        stream >> objectBuffer;
        QDataStream objectStream(&objectBuffer, QIODevice::ReadOnly);
        auto newObject = ModelObject::deserialize(objectStream, parent, ref, this);
        usedObjects.push_back(newObject.get());
        _pool.registerObj(std::move(newObject));
    }

    return std::move(usedObjects);
}

void ModelRoot::destroy() {
    _pool.destroy();
}
