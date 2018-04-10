#include "Ref.h"

using namespace AxiomModel;

SurfaceRef::SurfaceRef() : path({}) {}

SurfaceRef::SurfaceRef(std::vector<size_t> path) : path(std::move(path)) {}

NodeRef::NodeRef() : index(0) {}

NodeRef::NodeRef(AxiomModel::SurfaceRef surface, size_t index) : surface(std::move(surface)), index(index) {}

std::vector<size_t> NodeRef::path() const {
    auto newPath = std::vector<size_t>(surface.path);
    newPath.push_back(index);
    return newPath;
}

ControlRef::ControlRef() : index(0) {}

ControlRef::ControlRef(AxiomModel::NodeRef node, size_t index) : node(std::move(node)), index(index) {}

std::vector<size_t> ControlRef::path() const {
    auto newPath = node.path();
    newPath.push_back(index);
    return newPath;
}

QDataStream &AxiomModel::operator<<(QDataStream &stream, const AxiomModel::SurfaceRef &ref) {
    stream << (quint32) ref.path.size();
    for (const auto &crumb : ref.path) {
        stream << (quint32) crumb;
    }
    return stream;
}

QDataStream &AxiomModel::operator>>(QDataStream &stream, AxiomModel::SurfaceRef &ref) {
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
