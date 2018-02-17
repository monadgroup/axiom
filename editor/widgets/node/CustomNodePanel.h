#pragma once

#include <QtWidgets/QGraphicsObject>

class QGraphicsProxyWidget;
class QTextEdit;

namespace AxiomModel {

    class CustomNode;

}

namespace AxiomGui {

    class CustomNodePanel : public QGraphicsObject {
        Q_OBJECT

    public:

        AxiomModel::CustomNode *node;

        explicit CustomNodePanel(AxiomModel::CustomNode *node);

        QRectF boundingRect() const override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    private slots:

        void updateSize();

        void setOpen(bool open);

        void triggerUpdate();

        void triggerGeometryChange();

        void resizerChanged(QPointF topLeft, QPointF bottomRight);

    signals:

        void resizerSizeChanged(QSizeF newSize);

    private:

        QGraphicsProxyWidget *textProxy;
        QTextEdit *textEditor;

        bool eventFilter(QObject *object, QEvent *event) override;

    };

}
