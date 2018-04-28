#pragma once

#include <vector>
#include <QtCore/QDataStream>
#include <QtCore/QUuid>

namespace AxiomModel {

    struct SurfaceRef {
        QUuid root;
        std::vector<size_t> path;

        SurfaceRef();

        SurfaceRef(const QUuid &root);

        SurfaceRef(const QUuid &root, std::vector<size_t> path);
    };

    QDataStream &operator<<(QDataStream &stream, const SurfaceRef &ref);

    QDataStream &operator>>(QDataStream &stream, SurfaceRef &ref);

    struct NodeRef {
        SurfaceRef surface;
        size_t index;

        NodeRef();

        NodeRef(const QUuid &root);

        NodeRef(const SurfaceRef &surface, size_t index);

        std::vector<size_t> path() const;
    };

    QDataStream &operator<<(QDataStream &stream, const NodeRef &ref);

    QDataStream &operator>>(QDataStream &stream, NodeRef &ref);

    struct ControlRef {
        NodeRef node;
        size_t index;

        ControlRef();

        ControlRef(const QUuid &root);

        ControlRef(const NodeRef &node, size_t index);

        std::vector<size_t> path() const;
    };

    QDataStream &operator<<(QDataStream &stream, const ControlRef &ref);

    QDataStream &operator>>(QDataStream &stream, ControlRef &ref);

}
