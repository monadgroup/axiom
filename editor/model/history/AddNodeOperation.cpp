#include "AddNodeOperation.h"

#include "../Project.h"

using namespace AxiomModel;

AddNodeOperation::AddNodeOperation(AxiomModel::Project *project, AxiomModel::NodeRef nodeRef,
                                   AxiomModel::Node::Type nodeType, QPoint center, QByteArray nodeBuffer)
    : HistoryOperation(true, HistoryOperation::Type::ADD_NODE), project(project), nodeRef(std::move(nodeRef)),
      nodeType(nodeType), center(center), nodeBuffer(std::move(nodeBuffer)) {

}

std::unique_ptr<AddNodeOperation> AddNodeOperation::deserialize(QDataStream &stream, AxiomModel::Project *project) {
    NodeRef nodeRef; stream >> nodeRef;
    quint32 typeInt; stream >> typeInt;
    QPoint center; stream >> center;
    QByteArray nodeBuffer; stream >> nodeBuffer;

    return std::make_unique<AddNodeOperation>(project, nodeRef, (Node::Type) typeInt, center, std::move(nodeBuffer));
}

void AddNodeOperation::forward() {
    auto surface = project->findSurface(nodeRef.surface);
    QDataStream deserializeStream(&nodeBuffer, QIODevice::ReadOnly);
    surface->addFromStream(nodeType, nodeRef.index, deserializeStream, center);
}

void AddNodeOperation::backward() {
    auto node = project->findNode(nodeRef);
    node->removeWithoutOp();
}

void AddNodeOperation::serialize(QDataStream &stream) const {
    stream << nodeRef;
    stream << (quint32) nodeType;
    stream << center;
    stream << nodeBuffer;
}
