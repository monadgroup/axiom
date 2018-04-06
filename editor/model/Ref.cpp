#include "Ref.h"

using namespace AxiomModel;

SurfaceRef::SurfaceRef(std::vector<size_t> path) : path(std::move(path)) {}

NodeRef::NodeRef(AxiomModel::SurfaceRef surface, size_t index) : surface(std::move(surface)), index(index) {}

std::vector<size_t> NodeRef::path() const {
    auto newPath = std::vector<size_t>(surface.path);
    newPath.push_back(index);
    return newPath;
}

ControlRef::ControlRef(AxiomModel::NodeRef node, size_t index) : node(std::move(node)), index(index) {}

std::vector<size_t> ControlRef::path() const {
    auto newPath = node.path();
    newPath.push_back(index);
    return newPath;
}
