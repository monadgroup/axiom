#pragma once

#include "NodeSurface.h"

namespace AxiomModel {

    class GroupSurface : public NodeSurface {
    public:
        GroupSurface(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom, AxiomModel::ModelRoot *root);

        QString name() override { return "potatos"; }
    };

}
