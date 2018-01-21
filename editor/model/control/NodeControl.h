#pragma once

#include "../GridItem.h"
#include "../connection/ConnectionSink.h"

namespace AxiomModel {

    class Node;

    class NodeControl : public GridItem {
    Q_OBJECT

    public:
        Node *node;

        NodeControl(Node *node, QString name, QPoint pos, QSize size);

        virtual ConnectionSink *sink() const = 0;

        QString name() const { return m_name; }

        bool isMovable() const override { return true; }

    public slots:

        void setName(const QString &name);

    signals:

        void nameChanged(const QString &newName);

    protected:

        void initSink();

    private:

        QString m_name = "";

    private slots:

        void recalcSinkPos();
    };

}
