#include "DeleteNodeOperation.h"

#include <QtCore/QDataStream>
#include <QtCore/QIODevice>

using namespace AxiomModel;

DeleteNodeOperation::DeleteNodeOperation(AxiomModel::Project *project, AxiomModel::NodeRef nodeRef)
    : HistoryOperation(true), project(project), nodeRef(std::move(nodeRef)), nodeType(Node::Type::IO) {

}

void DeleteNodeOperation::forward() {
    auto node = project->findNode(nodeRef);

    // serialize the node to make undo easier
    nodeType = node->type;
    nodeBuffer = std::make_unique<QByteArray>();
    QDataStream serializeStream(nodeBuffer.get(), QIODevice::WriteOnly);
    node->serialize(serializeStream);

    node->removeWithoutOp();
}

void DeleteNodeOperation::backward() {
    auto surface = project->findSurface(nodeRef.surface);
    nodeRef.index = surface->items().size();

    QDataStream deserializeStream(nodeBuffer.get(), QIODevice::ReadOnly);

    // todo: somehow add at the correct index, and update indexes of everything after it?
    // otherwise the solution here is going to break on groupnodes :)
    surface->addFromStream(nodeType, deserializeStream);

    nodeBuffer.reset();
}
