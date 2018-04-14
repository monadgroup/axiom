#include <iostream>
#include "DeleteNodeOperation.h"

#include "../Project.h"

using namespace AxiomModel;

DeleteNodeOperation::DeleteNodeOperation(AxiomModel::Project *project, AxiomModel::NodeRef nodeRef)
    : HistoryOperation(true, Type::DELETE_NODE, false), project(project), nodeRef(std::move(nodeRef)),
      nodeType(Node::Type::IO) {

}

std::unique_ptr<DeleteNodeOperation> DeleteNodeOperation::deserialize(QDataStream &stream,
                                                                      AxiomModel::Project *project) {
    NodeRef nodeRef;
    stream >> nodeRef;
    quint32 typeInt;
    stream >> typeInt;
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
    node->serialize(serializeStream, QPoint(0, 0));

    node->removeWithoutOp();
}

void DeleteNodeOperation::backward() {
    auto surface = project->findSurface(nodeRef.surface);
    QDataStream deserializeStream(nodeBuffer.get(), QIODevice::ReadOnly);
    surface->addFromStream(nodeType, nodeRef.index, deserializeStream, QPoint(0, 0));

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
