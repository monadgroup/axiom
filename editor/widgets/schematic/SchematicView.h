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

    private slots:

        void newNode();

    protected:
        void contextMenuEvent(QContextMenuEvent *event) override;

        void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;

    private:
        AxiomModel::Schematic *schematic;
        QMenu *contextMenu;

        bool isPanning = false;
        QPoint lastMousePos;
    };

}
