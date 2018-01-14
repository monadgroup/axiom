#pragma once

#include "../GridSurface.h"

namespace AxiomModel {

    class Node;

    class NodeSurface : public GridSurface {
    Q_OBJECT

    public:
        Node *node;

        explicit NodeSurface(Node *node);

        bool locked() const { return m_locked; }

        static QPoint schematicToNodeSurface(QPoint p) { return p * 2; }
        static QSize schematicToNodeSurface(QSize s) { return s * 2; }
        static QPoint nodeSurfaceToSchematicFloor(QPoint p) { return {(int)floorf(p.x() / 2.f), (int)floorf(p.y() / 2.f)}; }
        static QPoint nodeSurfaceToSchematicCeil(QPoint p) { return {(int)ceilf(p.x() / 2.f), (int)ceilf(p.y() / 2.f)}; }
        static QSize nodeSurfaceToSchematicFloor(QSize s) { return {(int)floorf(s.width() / 2.f), (int)floorf(s.height() / 2.f)}; }
        static QSize nodeSurfaceToSchematicCeil(QSize s) { return {(int)ceilf(s.width() / 2.f), (int)ceilf(s.height() / 2.f)}; }

    public slots:

        void setLocked(bool newLocked);

    signals:

        void lockedChanged(bool locked);

    private slots:

        void setSize(QSize size);

    private:
        bool m_locked = true;
    };

}
