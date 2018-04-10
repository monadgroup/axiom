#pragma once

#include <vector>
#include <QtCore/QDataStream>

namespace AxiomModel {

    struct SurfaceRef {
        std::vector<size_t> path;

        SurfaceRef();
        explicit SurfaceRef(std::vector<size_t> path);
    };

    QDataStream &operator<<(QDataStream &stream, const SurfaceRef &ref);
    QDataStream &operator>>(QDataStream &stream, SurfaceRef &ref);

    struct NodeRef {
        SurfaceRef surface;
        size_t index;

        NodeRef();
        NodeRef(SurfaceRef surface, size_t index);

        std::vector<size_t> path() const;
    };

    QDataStream &operator<<(QDataStream &stream, const NodeRef &ref);
    QDataStream &operator>>(QDataStream &stream, NodeRef &ref);

    struct ControlRef {
        NodeRef node;
        size_t index;

        ControlRef();
        ControlRef(NodeRef node, size_t index);

        std::vector<size_t> path() const;
    };

    QDataStream &operator<<(QDataStream &stream, const ControlRef &ref);
    QDataStream &operator>>(QDataStream &stream, ControlRef &ref);

}
