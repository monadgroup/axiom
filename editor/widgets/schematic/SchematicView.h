#pragma once

#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QMenu>

namespace AxiomModel {
    class Schematic;
}

namespace AxiomGui {

    class SchematicPanel;

    class SchematicView : public QGraphicsView {
    Q_OBJECT

    public:
        explicit SchematicView(SchematicPanel *panel, AxiomModel::Schematic *schematic);

    protected:

        void mousePressEvent(QMouseEvent *event) override;

        void mouseMoveEvent(QMouseEvent *event) override;

        void mouseReleaseEvent(QMouseEvent *event) override;

        void wheelEvent(QWheelEvent *event) override;

    private slots:

        void pan(QPointF pan);

        void zoom(float zoom);

    private:
        AxiomModel::Schematic *schematic;

        bool isPanning = false;
        QPoint startMousePos;
        QPointF startPan;
        float lastScale = 1;

        static float zoomToScale(float zoom);
    };

}
