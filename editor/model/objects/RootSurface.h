#pragma once

#include "Surface.h"

namespace AxiomModel {

    class RootSurface : public Surface {
    public:
        RootSurface(const QUuid &uuid, QPointF pan, float zoom, AxiomModel::ModelRoot *root);

        QString name() override { return "Root"; }
    };

}
