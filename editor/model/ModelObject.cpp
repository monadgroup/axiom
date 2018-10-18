#include "ModelObject.h"

#include "../util.h"
#include "ModelRoot.h"
#include "ReferenceMapper.h"
#include "common/SequenceOperators.h"
#include "objects/Connection.h"
#include "objects/ControlSurface.h"
#include "objects/CustomNode.h"
#include "objects/NodeSurface.h"

using namespace AxiomModel;

ModelObject::ModelObject(ModelType modelType, const QUuid &uuid, const QUuid &parentUuid, ModelRoot *root)
    : PoolObject(uuid, parentUuid, &root->pool()), _modelType(modelType), _root(root) {}

AxiomCommon::BoxedSequence<ModelObject *> ModelObject::links() {
    return AxiomCommon::boxSequence(AxiomCommon::blank<ModelObject *>());
}

AxiomCommon::BoxedSequence<QUuid> ModelObject::deleteCompileLinks() {
    return AxiomCommon::boxSequence(AxiomCommon::once(parentUuid()));
}

AxiomCommon::BoxedSequence<QUuid> ModelObject::compileLinks() {
    return AxiomCommon::boxSequence(AxiomCommon::blank<QUuid>());
}

void ModelObject::remove() {
    removed();
    PoolObject::remove();
}
