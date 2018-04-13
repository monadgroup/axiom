#include "AddNodeOperation.h"

#include "../Project.h"
#include "../node/CustomNode.h"
#include "../node/GroupNode.h"

using namespace AxiomModel;

AddNodeOperation::AddNodeOperation(AxiomModel::Project *project, AxiomModel::NodeRef nodeRef, Node::Type type,
                                   QString name, QPoint pos)
    : HistoryOperation(true, Type::ADD_NODE), project(project), nodeRef(std::move(nodeRef)), type(type),
      name(std::move(name)), pos(pos) {

}

std::unique_ptr<AddNodeOperation> AddNodeOperation::deserialize(QDataStream &stream, Project *project) {
    NodeRef nodeRef;
    stream >> nodeRef;
    quint32 typeInt;
    stream >> typeInt;
    QString name;
    stream >> name;
    QPoint pos;
    stream >> pos;

    return std::make_unique<AddNodeOperation>(project, nodeRef, (Node::Type) typeInt, name, pos);
}

void AddNodeOperation::forward() {
    auto surface = project->findSurface(nodeRef.surface);
    assert(surface);

    auto defaultSize = QSize(3, 2);

    std::unique_ptr<Node> newNode;
    switch (type) {
        case Node::Type::CUSTOM:
            newNode = std::make_unique<CustomNode>(surface, name, pos, defaultSize);
            break;
        case Node::Type::GROUP:
            newNode = std::make_unique<GroupNode>(surface, name, pos, defaultSize);
            break;
        case Node::Type::IO:
            break;
    }

    if (!newNode) return;

    if (surface->runtime()) {
        newNode->createAndAttachRuntime(surface->runtime());
    }
    surface->insertItem(nodeRef.index, std::move(newNode));
}

void AddNodeOperation::backward() {
    auto node = project->findNode(nodeRef);
    if (node) node->removeWithoutOp();
}

void AddNodeOperation::serialize(QDataStream &stream) const {
    stream << nodeRef;
    stream << (quint32) type;
    stream << name;
    stream << pos;
}
