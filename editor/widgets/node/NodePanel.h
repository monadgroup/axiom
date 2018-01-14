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

        NodePanel(AxiomModel::Node *node);
    };

}
