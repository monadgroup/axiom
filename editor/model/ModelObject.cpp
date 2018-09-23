#include "ModelObject.h"

#include "../util.h"
#include "ModelRoot.h"
#include "ReferenceMapper.h"
#include "SequenceOperators.h"
#include "objects/Connection.h"
#include "objects/ControlSurface.h"
#include "objects/CustomNode.h"
#include "objects/NodeSurface.h"

using namespace AxiomModel;

ModelObject::ModelObject(ModelType modelType, const QUuid &uuid, const QUuid &parentUuid, ModelRoot *root)
    : PoolObject(uuid, parentUuid, &root->pool()), _modelType(modelType), _root(root) {}

Sequence<ModelObject *> ModelObject::links() {
    return blank<ModelObject *>();
}

Sequence<QUuid> ModelObject::deleteCompileLinks() {
    return oneShot(parentUuid());
}

Sequence<QUuid> ModelObject::compileLinks() {
    return blank<QUuid>();
}

void ModelObject::remove() {
    removed.trigger();
    PoolObject::remove();
}
