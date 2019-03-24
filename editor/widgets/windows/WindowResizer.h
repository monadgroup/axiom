#pragma once

#include <QtWidgets/QWidget>

namespace AxiomGui {

    class WindowResizer : public QWidget {
        Q_OBJECT

    public:
        explicit WindowResizer(QWidget *parent = nullptr);

    signals:
        void startResize();
        void resized(QPoint amount);

    protected:
        void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;

    private:
        bool isDragging;
        QPointF startMousePos;
    };
}
