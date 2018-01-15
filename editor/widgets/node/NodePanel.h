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

    private slots:

        void lockToggleChanged(int state);

        void openToggleChanged(int state);

    private:

        QWidget *wrapperWidget;
        QString stylesheet;
    };

}
