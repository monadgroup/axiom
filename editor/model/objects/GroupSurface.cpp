#include "GroupSurface.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "GroupNode.h"
#include "editor/compiler/interface/Runtime.h"

using namespace AxiomModel;

GroupSurface::GroupSurface(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom,
                           AxiomModel::ModelRoot *root)
    : NodeSurface(uuid, parentUuid, pan, zoom, root), _node(find<GroupNode *>(root->nodes(), parentUuid)) {
    _node->nameChanged.connect(&nameChanged);
}

std::unique_ptr<GroupSurface> GroupSurface::create(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom,
                                                   AxiomModel::ModelRoot *root) {
    return std::make_unique<GroupSurface>(uuid, parentUuid, pan, zoom, root);
}

QString GroupSurface::name() {
    return _node->name();
}

void GroupSurface::attachRuntime(MaximCompiler::Runtime *runtime, MaximCompiler::Transaction *transaction) {
    if (runtime) {
        runtimeId = runtime->nextId();
    } else {
        runtimeId = 0;
    }
    NodeSurface::attachRuntime(runtime, transaction);
}
