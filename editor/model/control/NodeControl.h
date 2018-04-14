#pragma once

#include "../GridItem.h"
#include "../connection/ConnectionSink.h"
#include "../Ref.h"
#include "compiler/common/ControlType.h"

namespace MaximRuntime {

    class Control;

}

namespace AxiomModel {

    class Node;

    class NodeControl : public GridItem {
    Q_OBJECT

    public:
        Node *node;

        NodeControl(Node *node, QString name, QPoint pos, QSize size);

        //static std::unique_ptr<NodeControl> fromRuntimeControl(Node *node, MaximRuntime::Control *runtime);

        static std::unique_ptr<NodeControl> create(Node *node, MaximCommon::ControlType type, QString name);

        ControlRef ref() const;

        virtual ConnectionSink *sink() = 0;

        MaximRuntime::Control *runtime() const { return _runtime; }

        void attachRuntime(MaximRuntime::Control *runtime);

        QString name() const { return _name; }

        virtual MaximCommon::ControlType type() const = 0;

        bool showName() const { return m_showName; }

        bool isMovable() const override { return true; }

        bool isDeletable() const override { return false; }

        std::unique_ptr<GridItem> clone(GridSurface *newParent, QPoint newPos, QSize newSize) const override;

        NodeControl *exposeBase() const { return _exposeBase; }

    public slots:

        void setShowName(bool showName);

        void setShowNameNoOp(bool showName);

        void setExposeBase(NodeControl *base);

        void saveValue() override = 0;

        void restoreValue() override = 0;

        void startResize();

        void finishResize();

        void startDragging() override;

        void finishDragging() override;

        void serialize(QDataStream &stream, QPoint offset) const override;

        void deserialize(QDataStream &stream, QPoint offset) override;

    signals:

        void showNameChanged(bool newShowName);

        void exposeBaseChanged(NodeControl *newBase);

    protected:

        void initSink();

    private:

        bool m_showName = true;
        NodeControl *_exposeBase = nullptr;
        MaximRuntime::Control *_runtime = nullptr;
        QString _name;

        QPoint startDragPos;
        QPoint startResizeTopLeft;
        QPoint startResizeBottomRight;

    private slots:

        void recalcSinkPos();
    };

}
