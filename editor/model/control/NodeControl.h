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

        bool showName() const { return m_showName; }

        bool isMovable() const override { return true; }

    public slots:

        void setName(const QString &name);

        void setShowName(bool showName);

    signals:

        void nameChanged(const QString &newName);

        void showNameChanged(bool newShowName);

    protected:

        void initSink();

    private:

        QString m_name = "";
        bool m_showName = true;

    private slots:

        void recalcSinkPos();
    };

}
