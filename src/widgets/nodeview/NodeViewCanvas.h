#pragma once
#include <QtWidgets/QGraphicsView>

namespace AxiomGui {

    class NodeViewCanvas : public QGraphicsView {
        Q_OBJECT

    public:
        static QSize gridSize;

        explicit NodeViewCanvas(QWidget *parent = nullptr);

    protected:
        void drawBackground(QPainter *painter, const QRectF &rect) override;
        void resizeEvent(QResizeEvent *event) override;
    };

}
