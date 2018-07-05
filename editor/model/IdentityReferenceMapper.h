#pragma once

#include "ReferenceMapper.h"

namespace AxiomModel {

    class IdentityReferenceMapper : public ReferenceMapper {
    public:
        QUuid mapUuid(const QUuid &input) override { return input; }

        QPoint mapPos(const QUuid &parent, const QPoint &input) override { return input; }
    };
}
