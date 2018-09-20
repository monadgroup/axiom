#pragma once

#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QMenu>
#include <memory>
#include <set>

#include "common/Hookable.h"
#include "editor/model/ConnectionWire.h"
#include "editor/model/objects/Node.h"
#include "editor/model/objects/PortalControl.h"

namespace AxiomModel {
    class NodeSurface;

    class Control;

    class ConnectionWire;

    class GridItem;
}

namespace AxiomGui {

    class IConnectable;

    class NodeSurfacePanel;

    class NodeSurfaceCanvas : public QGraphicsScene, public AxiomCommon::Hookable {
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

        static QPointF nodeRealPos(const QPointF &p);

        static QSize nodeRealSize(const QSize &s);

        static QPoint controlRealPos(const QPoint &p);

        static QPointF controlRealPos(const QPointF &p);

        static QSize controlRealSize(const QSize &s);

    public slots:

        void startConnecting(IConnectable *control);

        void updateConnecting(QPointF mousePos);

        void endConnecting(QPointF mousePos);

    protected:
        void drawBackground(QPainter *painter, const QRectF &rect) override;

        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    private slots:

        void addNode(AxiomModel::Node *node);

        void newNode(QPointF scenePos, QString name, AxiomModel::Node::NodeType type,
                     AxiomModel::ConnectionWire::WireType portalWireType,
                     AxiomModel::PortalControl::PortalType portalType);

        void addWire(AxiomModel::ConnectionWire *wire);

        void doRuntimeUpdate();

    private:
        bool isSelecting = false;
        std::set<AxiomModel::GridItem *> lastSelectedItems;

        QVector<QPointF> selectionPoints;
        QGraphicsPathItem *selectionPath;

        std::unique_ptr<AxiomModel::ConnectionWire> connectionWire;
        AxiomModel::Control *sourceControl;

        void leftMousePressEvent(QGraphicsSceneMouseEvent *event);

        void leftMouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        static void drawGrid(QPainter *painter, const QRectF &rect, const QSize &size, const QColor &color,
                             qreal pointSize);
    };
}
