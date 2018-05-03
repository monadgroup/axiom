#pragma once

#include "Surface.h"

namespace AxiomModel {

    class GroupSurface : public Surface {
    public:
        GroupSurface(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom, AxiomModel::ModelRoot *root);
    };

}
