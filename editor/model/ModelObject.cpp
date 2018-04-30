#include "ModelObject.h"

#include "ModelRoot.h"
#include "../util.h"

using namespace AxiomModel;

ModelObject::ModelObject(ModelType modelType, const QUuid &uuid, const QUuid &parentUuid, ModelRoot *root)
    : PoolObject(uuid, parentUuid, root->pool()), _modelType(modelType), _root(root) {
}

std::unique_ptr<ModelObject> ModelObject::deserialize(QDataStream &stream, ModelRoot *root) {
    uint8_t typeInt; stream >> typeInt;
    QUuid uuid; stream >> uuid;
    QUuid parentUuid; stream >> parentUuid;

    auto type = (ModelType) typeInt;
    switch (type) {
        case ModelType::SURFACE:break;
        case ModelType::NODE:break;
        case ModelType::CONTROL:break;
        case ModelType::LIBRARY_ENTRY:break;
        case ModelType::HISTORY_ACTION:break;
    }

    return nullptr;
}

void ModelObject::serialize(QDataStream &stream) const {
    stream << (uint8_t) modelType();
    stream << uuid();
    stream << parentUuid();
}
