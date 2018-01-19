#pragma once

#include <QtCore/QObject>

#include "Node.h"

namespace AxiomModel {

    class CustomNode : public Node {
    Q_OBJECT

    public:
        CustomNode(Schematic *parent, QPoint pos, QSize);
    };

}
