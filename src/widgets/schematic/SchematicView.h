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

    private slots:
        void newNode();

    protected:
        void resizeEvent(QResizeEvent *event) override;
        void contextMenuEvent(QContextMenuEvent *event) override;

    private:
        AxiomModel::Schematic *schematic;
        QMenu *contextMenu;
    };

}
