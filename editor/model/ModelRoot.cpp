#include "ModelRoot.h"

#include <cassert>

#include "ModelObject.h"

using namespace AxiomModel;

ModelRoot::ModelRoot(AxiomModel::Pool *pool) : _pool(pool) {

}

void ModelRoot::deserializeChunk(QDataStream &stream, std::vector<std::unique_ptr<ModelObject>> &objects) {
    uint32_t magic; stream >> magic;
    assert(magic == schemaMagic);
    uint32_t version; stream >> version;
    assert(version >= minSchemaVersion && version <= schemaVersion);

    uint32_t objectCount; stream >> objectCount;
    for (uint32_t i = 0; i < objectCount; i++) {
        QByteArray objectBuffer; stream >> objectBuffer;
        QDataStream objectStream(&objectBuffer, QIODevice::ReadOnly);

        auto newObj = ModelObject::deserialize(stream, this);
        if (!newObj) continue;
        objects.push_back(std::move(newObj));
    }
}
