#pragma once
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGraphicsScene>

namespace AxiomGui {

    class SchematicPanel : public QDockWidget {
        Q_OBJECT

    public:
        explicit SchematicPanel(QWidget *parent = nullptr);

    private:
        QGraphicsScene *scene;
    };

}
