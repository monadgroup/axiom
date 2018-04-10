#include "DeleteNodeOperation.h"

#include <QtCore/QDataStream>
#include <QtCore/QIODevice>

#include "../Project.h"

using namespace AxiomModel;

DeleteNodeOperation::DeleteNodeOperation(AxiomModel::Project *project, AxiomModel::NodeRef nodeRef)
    : HistoryOperation(true, Type::DELETE_NODE), project(project), nodeRef(std::move(nodeRef)), nodeType(Node::Type::IO) {

}

std::unique_ptr<DeleteNodeOperation> DeleteNodeOperation::deserialize(QDataStream &stream,
                                                                      AxiomModel::Project *project) {
    NodeRef nodeRef; stream >> nodeRef;
    quint32 typeInt; stream >> typeInt;
    auto nodeBuffer = std::make_unique<QByteArray>();
    stream >> *nodeBuffer;

    auto result = std::make_unique<DeleteNodeOperation>(project, nodeRef);
    result->nodeType = (Node::Type) typeInt;
    if (nodeBuffer->size()) {
        result->nodeBuffer = std::move(nodeBuffer);
    }
    return result;
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

void DeleteNodeOperation::serialize(QDataStream &stream) const {
    stream << nodeRef;
    stream << (quint32) nodeType;

    if (nodeBuffer) {
        stream << *nodeBuffer;
    } else {
        stream << (quint32) 0;
    }
}
