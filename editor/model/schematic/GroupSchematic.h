#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>

#include "Schematic.h"

namespace AxiomModel {

    class GroupNode;

    class GroupSchematic : public Schematic {
    Q_OBJECT

    public:
        GroupNode *node;

        explicit GroupSchematic(GroupNode *node);

        QString name() override;

        bool canExposeControl() override { return true; }

        void exposeControl(AxiomModel::NodeControl *control) override;
    };

}
