#pragma once

#include <vector>

namespace AxiomModel {

    struct SurfaceRef {
        std::vector<size_t> path;

        explicit SurfaceRef(std::vector<size_t> path);
    };

    struct NodeRef {
        SurfaceRef surface;
        size_t index;

        NodeRef(SurfaceRef surface, size_t index);

        std::vector<size_t> path() const;
    };

    struct ControlRef {
        NodeRef node;
        size_t index;

        ControlRef(NodeRef node, size_t index);

        std::vector<size_t> path() const;
    };

}
