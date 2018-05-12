#include "ModelRoot.h"

#include <cassert>

#include "ModelObject.h"
#include "Pool.h"
#include "WatchSequenceOperators.h"

#include "objects/NodeSurface.h"
#include "objects/Node.h"
#include "objects/ControlSurface.h"
#include "objects/Control.h"
#include "objects/Connection.h"

using namespace AxiomModel;

ModelRoot::ModelRoot() : _nodeSurfaces(dynamicCastWatch<NodeSurface*>(_pool.sequence())),
                         _nodes(dynamicCastWatch<Node*>(_pool.sequence())),
                         _controlSurfaces(dynamicCastWatch<ControlSurface*>(_pool.sequence())),
                         _controls(dynamicCastWatch<Control*>(_pool.sequence())),
                         _connections(dynamicCastWatch<Connection*>(_pool.sequence())) {}

ModelRoot::ModelRoot(QDataStream &stream) : ModelRoot() {
    deserializeChunk(stream, QUuid());
    _history = HistoryList(stream, this);
}

void ModelRoot::serialize(QDataStream &stream) {
    serializeChunk(stream, QUuid(), dynamicCast<ModelObject*>(_pool.sequence()));
    _history.serialize(stream);
}

void ModelRoot::deserializeChunk(QDataStream &stream, const QUuid &parent) {
    uint32_t objectCount; stream >> objectCount;
    for (uint32_t i = 0; i < objectCount; i++) {
        QByteArray objectBuffer; stream >> objectBuffer;
        QDataStream objectStream(&objectBuffer, QIODevice::ReadOnly);
        _pool.registerObj(ModelObject::deserialize(objectStream, parent, this));
    }
}

void ModelRoot::destroy() {
    _pool.destroy();
}
