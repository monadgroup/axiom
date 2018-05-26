#pragma once

#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QMenu>

#include "common/Hookable.h"
#include "editor/model/actions/CompositeAction.h"

namespace AxiomModel {
    class NodeSurface;
}

namespace AxiomGui {

    class NodeSurfacePanel;

    class NodeSurfaceCanvas;

    class NodeSurfaceView : public QGraphicsView, public AxiomCommon::Hookable {
    Q_OBJECT

    public:
        explicit NodeSurfaceView(NodeSurfacePanel *panel, AxiomModel::NodeSurface *surface);

    protected:

        void mousePressEvent(QMouseEvent *event) override;

        void mouseMoveEvent(QMouseEvent *event) override;

        void mouseReleaseEvent(QMouseEvent *event) override;

        void resizeEvent(QResizeEvent *event) override;

        void wheelEvent(QWheelEvent *event) override;

        void dragEnterEvent(QDragEnterEvent *event) override;

        void dragMoveEvent(QDragMoveEvent *event) override;

        void dragLeaveEvent(QDragLeaveEvent *event) override;

        void dropEvent(QDropEvent *event) override;

        void focusInEvent(QFocusEvent *event) override;

    private slots:

        void pan(QPointF pan);

        void zoom(float zoom);

        void deleteSelected();

        void selectAll();

        void cutSelected();

        void copySelected();

        void pasteBuffer();

        void doUndo();

        void doRedo();

    private:
        AxiomModel::NodeSurface *surface;

        bool isPanning = false;
        QPoint startMousePos;
        QPointF startPan;
        float lastScale = 1;
        std::unique_ptr<AxiomModel::CompositeAction> dragAndDropAction;
        bool dragAndDropRebuild = false;

        static float zoomToScale(float zoom);

        void updateHistoryState();
    };

}
