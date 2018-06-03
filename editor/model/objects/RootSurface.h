#pragma once

#include "NodeSurface.h"

namespace AxiomModel {

    class RootSurface : public NodeSurface {
    public:
        RootSurface(const QUuid &uuid, QPointF pan, float zoom, AxiomModel::ModelRoot *root);

        QString name() override { return "Root"; }

        bool canExposeControl() const override { return false; }

        bool canHaveAutomation() const override { return true; }
    };

}
