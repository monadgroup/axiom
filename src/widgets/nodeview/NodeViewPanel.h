#pragma once
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGraphicsScene>

namespace AxiomGui {

    class NodeViewPanel : public QDockWidget {
        Q_OBJECT

    public:
        explicit NodeViewPanel(QWidget *parent = nullptr);

    private:
        QGraphicsScene *scene;
    };

}
