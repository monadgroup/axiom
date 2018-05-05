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

ModelRoot::ModelRoot(AxiomModel::Pool *pool) : _pool(pool), _nodeSurfaces(filterType<NodeSurface*>(*pool)),
                                               _nodes(filterType<Node*>(*pool)),
                                               _controlSurfaces(filterType<ControlSurface*>(*pool)),
                                               _controls(filterType<Control*>(*pool)),
                                               _connections(filterType<Connection*>(*pool)) {

}

void ModelRoot::deserializeChunk(QDataStream &stream, const QUuid &parent, std::vector<std::unique_ptr<ModelObject>> &objects) {
    uint32_t magic; stream >> magic;
    assert(magic == schemaMagic);
    uint32_t version; stream >> version;
    assert(version >= minSchemaVersion && version <= schemaVersion);

    uint32_t objectCount; stream >> objectCount;
    for (uint32_t i = 0; i < objectCount; i++) {
        QByteArray objectBuffer; stream >> objectBuffer;
        QDataStream objectStream(&objectBuffer, QIODevice::ReadOnly);

        auto newObj = ModelObject::deserialize(stream, parent, this);
        if (!newObj) continue;
        objects.push_back(std::move(newObj));
    }
}
