#include "GroupSurface.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "GroupNode.h"
#include "editor/compiler/interface/Runtime.h"

using namespace AxiomModel;

GroupSurface::GroupSurface(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom,
                           AxiomModel::ModelRoot *root)
    : NodeSurface(uuid, parentUuid, pan, zoom, root),
      _node(find(AxiomCommon::dynamicCast<GroupNode *>(root->nodes().sequence()), parentUuid)) {
    _node->nameChanged.forward(&nameChanged);
}

std::unique_ptr<GroupSurface> GroupSurface::create(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom,
                                                   AxiomModel::ModelRoot *root) {
    return std::make_unique<GroupSurface>(uuid, parentUuid, pan, zoom, root);
}

QString GroupSurface::name() {
    return _node->name();
}

AxiomCommon::BoxedSequence<QUuid> GroupSurface::compileLinks() {
    return AxiomCommon::boxSequence(AxiomCommon::flatten(std::array<AxiomCommon::BoxedSequence<QUuid>, 2>{
        AxiomCommon::boxSequence(AxiomCommon::once(node()->surface()->uuid())), node()->surface()->compileLinks()}));
}

void GroupSurface::attachRuntime(MaximCompiler::Runtime *runtime, MaximCompiler::Transaction *transaction) {
    if (runtime) {
        runtimeId = runtime->nextId();
    } else {
        runtimeId = 0;
    }
    NodeSurface::attachRuntime(runtime, transaction);
}
