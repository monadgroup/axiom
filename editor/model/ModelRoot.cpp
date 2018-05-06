#include "ModelRoot.h"

#include <cassert>

#include "ModelObject.h"
#include "Pool.h"
#include "CollectionViewOperators.h"

#include "objects/NodeSurface.h"
#include "objects/Node.h"
#include "objects/ControlSurface.h"
#include "objects/Control.h"
#include "objects/Connection.h"

using namespace AxiomModel;

ModelRoot::ModelRoot() : _nodeSurfaces(filterType<NodeSurface*>(_pool)),
                         _nodes(filterType<Node*>(_pool)),
                         _controlSurfaces(filterType<ControlSurface*>(_pool)),
                         _controls(filterType<Control*>(_pool)),
                         _connections(filterType<Connection*>(_pool)) {

}

ModelRoot::ModelRoot(QDataStream &stream) : ModelRoot() {
    deserializeChunk(stream, QUuid());
    _history = HistoryList(stream, this);
}

void ModelRoot::serialize(QDataStream &stream) {
    serializeChunk(stream, QUuid(), filterType<ModelObject*>(_pool));
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
