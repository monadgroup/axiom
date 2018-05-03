#include "Node.h"

#include "Surface.h"
#include "../ModelRoot.h"
#include "../Pool.h"

using namespace AxiomModel;

Node::Node(ModelType type, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, AxiomModel::ModelRoot *root)
    : GridItem(&find(root->surfaces(), parentUuid)->grid, pos, size, selected), ModelObject(type, uuid, parentUuid, root) {

}

void Node::deserialize(QDataStream &stream, QPoint &pos, QSize &size, bool &selected) const {
    GridItem::deserialize(stream, pos, size, selected);
}

void Node::serialize(QDataStream &stream) const {
    GridItem::serialize(stream);
}
