#pragma once

#include "../GridItem.h"

namespace AxiomModel {

    class Node;

    class NodeControl : public GridItem {
    Q_OBJECT

    public:
        enum class Channel {
            LEFT = 1 << 0,
            RIGHT = 1 << 1,
            BOTH = LEFT | RIGHT
        };

        const Channel channel;

        Node *node;

        NodeControl(Node *node, Channel channel);

        QString name() const { return m_name; }

        QPoint worldPos() const { return m_worldPos; }

        bool isMovable() const override { return true; }

    public slots:

        void setName(const QString &name);

    signals:

        void nameChanged(const QString &newName);

        void worldPosChanged(QPoint newPos);

    private:
        QString m_name = "";

        QPoint m_worldPos;

    private slots:

        void recalcWorldPos();
    };

}
