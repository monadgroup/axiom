#include <iostream>
#include "DeleteNodeOperation.h"

#include "../Project.h"

using namespace AxiomModel;

DeleteNodeOperation::DeleteNodeOperation(AxiomModel::Project *project, AxiomModel::NodeRef nodeRef, Node::Type nodeType, QByteArray nodeBuffer)
    : HistoryOperation(true, HistoryOperation::ReadLevel::NODE, Type::DELETE_NODE), project(project), nodeRef(std::move(nodeRef)),
      nodeType(nodeType), nodeBuffer(std::move(nodeBuffer)) {

}

std::unique_ptr<DeleteNodeOperation> DeleteNodeOperation::deserialize(QDataStream &stream,
                                                                      AxiomModel::Project *project) {
    NodeRef nodeRef;
    stream >> nodeRef;
    quint32 typeInt;
    stream >> typeInt;
    QByteArray nodeBuffer;
    stream >> nodeBuffer;

    return std::make_unique<DeleteNodeOperation>(project, nodeRef, (Node::Type) typeInt, std::move(nodeBuffer));
}

void DeleteNodeOperation::forward() {
    auto node = project->findNode(nodeRef);
    node->removeWithoutOp();
}

void DeleteNodeOperation::backward() {
    auto surface = project->findSurface(nodeRef.surface);
    QDataStream deserializeStream(&nodeBuffer, QIODevice::ReadOnly);
    surface->addFromStream(nodeType, nodeRef.index, deserializeStream, QPoint(0, 0));
}

void DeleteNodeOperation::serialize(QDataStream &stream) const {
    stream << nodeRef;
    stream << (quint32) nodeType;
    stream << nodeBuffer;
}
