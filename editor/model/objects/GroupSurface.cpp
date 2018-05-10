#include "GroupSurface.h"

#include "GroupNode.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"

using namespace AxiomModel;

GroupSurface::GroupSurface(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom,
                           AxiomModel::ModelRoot *root)
    : NodeSurface(uuid, parentUuid, pan, zoom, root), _node(find<GroupNode*>(root->nodes(), parentUuid)) {
    _node->nameChanged.connect(&nameChanged);
}

std::unique_ptr<GroupSurface> GroupSurface::create(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom,
                                                   AxiomModel::ModelRoot *root) {
    return std::make_unique<GroupSurface>(uuid, parentUuid, pan, zoom, root);
}

QString GroupSurface::name() {
    return _node->name();
}
