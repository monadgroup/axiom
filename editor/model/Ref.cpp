#include "Ref.h"

using namespace AxiomModel;

SurfaceRef::SurfaceRef() {}

SurfaceRef::SurfaceRef(const QUuid &root) : root(root), path({}) {}

SurfaceRef::SurfaceRef(const QUuid &root, std::vector<size_t> path) : root(root), path(std::move(path)) {}

NodeRef::NodeRef() : index(0) {}

NodeRef::NodeRef(const QUuid &root) : surface(root), index(0) {}

NodeRef::NodeRef(const AxiomModel::SurfaceRef &surface, size_t index) : surface(surface), index(index) {}

std::vector<size_t> NodeRef::path() const {
    auto newPath = std::vector<size_t>(surface.path);
    newPath.push_back(index);
    return newPath;
}

ControlRef::ControlRef() : index(0) {}

ControlRef::ControlRef(const QUuid &root) : node(root), index(0) {}

ControlRef::ControlRef(const AxiomModel::NodeRef &node, size_t index) : node(node), index(index) {}

std::vector<size_t> ControlRef::path() const {
    auto newPath = node.path();
    newPath.push_back(index);
    return newPath;
}

QDataStream &AxiomModel::operator<<(QDataStream &stream, const AxiomModel::SurfaceRef &ref) {
    stream << ref.root;
    stream << (quint32) ref.path.size();
    for (const auto &crumb : ref.path) {
        stream << (quint32) crumb;
    }
    return stream;
}

QDataStream &AxiomModel::operator>>(QDataStream &stream, AxiomModel::SurfaceRef &ref) {
    stream >> ref.root;
    ref.path.clear();
    quint32 pathCount;
    stream >> pathCount;
    for (quint32 i = 0; i < pathCount; i++) {
        quint32 crumb;
        stream >> crumb;
        ref.path.push_back(crumb);
    }
    return stream;
}

QDataStream &AxiomModel::operator<<(QDataStream &stream, const AxiomModel::NodeRef &ref) {
    stream << ref.surface;
    stream << (quint32) ref.index;
    return stream;
}

QDataStream &AxiomModel::operator>>(QDataStream &stream, AxiomModel::NodeRef &ref) {
    stream >> ref.surface;
    quint32 index;
    stream >> index;
    ref.index = index;
    return stream;
}

QDataStream &AxiomModel::operator<<(QDataStream &stream, const AxiomModel::ControlRef &ref) {
    stream << ref.node;
    stream << (quint32) ref.index;
    return stream;
}

QDataStream &AxiomModel::operator>>(QDataStream &stream, AxiomModel::ControlRef &ref) {
    stream >> ref.node;
    quint32 index;
    stream >> index;
    ref.index = index;
    return stream;
}
