#pragma once

#include <QtCore/QObject>

#include "../GridItem.h"
#include "../connection/ConnectionSink.h"

namespace AxiomModel {

    class Schematic;

    class JunctionNode : public GridItem {
        Q_OBJECT

    public:
        ConnectionSink sink;

        JunctionNode(Schematic *parent, QPoint pos);

        bool isMovable() const override { return true; }

        bool isResizable() const override { return false; }
    };

}
