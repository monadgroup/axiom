#pragma once

#include "ReferenceMapper.h"

namespace AxiomModel {

    class IdentityReferenceMapper : public ReferenceMapper {
    public:
        QUuid map(const QUuid &input) override {
            return input;
        }
    };

}
