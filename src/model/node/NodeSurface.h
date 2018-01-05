#pragma once

#include "../GridSurface.h"

namespace AxiomModel {

    class Node;

    class NodeSurface : public GridSurface {
    Q_OBJECT

    public:
        Node *node;

        explicit NodeSurface(Node *node);

    private slots:

        void setSize(QSize size);
    };

}
