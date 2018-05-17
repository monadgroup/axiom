#pragma once

#include <memory>
#include <set>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QMenu>

#include "editor/model/Hookable.h"
#include "editor/model/ConnectionWire.h"

namespace AxiomModel {
    class NodeSurface;

    class Node;

    class Control;

    class ConnectionWire;

    class GridItem;
}

namespace AxiomGui {

    class IConnectable;

    class NodeSurfacePanel;

    class NodeSurfaceCanvas : public QGraphicsScene, public AxiomModel::Hookable {
    Q_OBJECT

    public:
        static QSize nodeGridSize;
        static QSize controlGridSize;
        static int wireZVal;
        static int activeWireZVal;
        static int nodeZVal;
        static int activeNodeZVal;
        static int panelZVal;
        static int selectionZVal;

        NodeSurfacePanel *panel;

        AxiomModel::NodeSurface *surface;

        explicit NodeSurfaceCanvas(NodeSurfacePanel *panel, AxiomModel::NodeSurface *surface);

        static QPoint nodeRealPos(const QPoint &p);

        static QSize nodeRealSize(const QSize &s);

        static QPoint controlRealPos(const QPoint &p);

        static QPointF controlRealPos(const QPointF &p);

        static QSize controlRealSize(const QSize &s);

    public slots:

        void startConnecting(IConnectable *control);

        void updateConnecting(QPointF mousePos);

        void endConnecting(QPointF mousePos);

        void cancelConnecting();

    protected:
        void drawBackground(QPainter *painter, const QRectF &rect) override;

        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    private slots:

        void addNode(AxiomModel::Node *node);

        void newNode(QPointF scenePos, QString name, bool group);

        void addWire(AxiomModel::ConnectionWire *wire);

        void deleteSelected();

        void selectAll();

        void doRuntimeUpdate();

    private:
        bool isSelecting = false;
        std::set<AxiomModel::GridItem *> lastSelectedItems;

        QVector<QPointF> selectionPoints;
        QGraphicsPathItem *selectionPath;

        std::optional<AxiomModel::ConnectionWire> connectionWire;
        AxiomModel::Control *sourceControl;

        //bool isConnecting = false;
        //AxiomModel::ConnectionWire connectionWire;
        //AxiomModel::Control *sourceControl;

        void leftMousePressEvent(QGraphicsSceneMouseEvent *event);

        void leftMouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        static void
        drawGrid(QPainter *painter, const QRectF &rect, const QSize &size, const QColor &color, qreal pointSize);
    };

}
