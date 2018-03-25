#pragma once

#include "../GridItem.h"
#include "../connection/ConnectionSink.h"

namespace MaximRuntime {

    class Control;

}

namespace AxiomModel {

    class Node;

    class NodeControl : public GridItem {
    Q_OBJECT

    public:
        Node *node;

        NodeControl(Node *node, MaximRuntime::Control *runtime, QPoint pos, QSize size);

        static std::unique_ptr<NodeControl> fromRuntimeControl(Node *node, MaximRuntime::Control *runtime);

        virtual ConnectionSink *sink() = 0;

        MaximRuntime::Control *runtime() const { return _runtime; }

        QString name() const;

        bool showName() const { return m_showName; }

        bool isMovable() const override { return true; }

        bool isDeletable() const override { return false; }

        std::unique_ptr<GridItem> clone(GridSurface *newParent, QPoint newPos, QSize newSize) const override;

        NodeControl *exposeBase() const { return _exposeBase; }

    public slots:

        void setShowName(bool showName);

        void setExposeBase(NodeControl *base);

    signals:

        void showNameChanged(bool newShowName);

        void exposeBaseChanged(NodeControl *newBase);

    protected:

        void initSink();

    private:

        bool m_showName = true;
        NodeControl *_exposeBase = nullptr;
        MaximRuntime::Control *_runtime;

    private slots:

        void recalcSinkPos();
    };

}
