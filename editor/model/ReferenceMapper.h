#pragma once

#include <QtCore/QUuid>

namespace AxiomModel {

    class ReferenceMapper {
    public:
        virtual ~ReferenceMapper() = default;

        virtual QUuid map(const QUuid &input) = 0;
    };

}
