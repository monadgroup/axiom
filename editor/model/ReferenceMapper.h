#pragma once

#include <QtCore/QUuid>
#include <QtCore/QPoint>

namespace AxiomModel {

    class ReferenceMapper {
    public:
        virtual ~ReferenceMapper() = default;

        virtual QUuid mapUuid(const QUuid &input) = 0;

        virtual QPoint mapPos(const QUuid &parent, const QPoint &input) = 0;
    };

}
