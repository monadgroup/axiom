#pragma once

#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QMenu>

namespace AxiomModel {
    class Schematic;
}

namespace AxiomGui {

    class SchematicView : public QGraphicsView {
    Q_OBJECT

    public:
        explicit SchematicView(AxiomModel::Schematic *schematic, QWidget *parent = nullptr);

    public slots:

        void pan(QPointF delta);

    protected:

        void mousePressEvent(QMouseEvent *event) override;

        void mouseMoveEvent(QMouseEvent *event) override;

        void mouseReleaseEvent(QMouseEvent *event) override;

    private:
        AxiomModel::Schematic *schematic;

        bool isPanning = false;
        QPoint lastMousePos;
    };

}
