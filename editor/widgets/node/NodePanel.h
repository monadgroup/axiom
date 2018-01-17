#pragma once

#include <QtWidgets/QWidget>

namespace AxiomModel {
    class Node;
}

namespace AxiomGui {

    class NodePanel : public QWidget {
    Q_OBJECT

    public:
        AxiomModel::Node *node;

        explicit NodePanel(AxiomModel::Node *node);

    public slots:

        void setNodeHover(bool hover) {
            visibleFromNodeHover = hover;
            updateVisible();
        }

        void setSelfHover(bool hover) {
            visibleFromSelfHover = hover;
            updateVisible();
        }

    protected:

        void enterEvent(QEvent *event) override;

        void leaveEvent(QEvent *event) override;

    private slots:

        void openToggleChanged(int state);

        void updateVisible();

    private:
        bool visibleFromNodeHover = false;
        bool visibleFromSelfHover = false;

        QWidget *wrapperWidget;
    };

}
