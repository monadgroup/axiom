#pragma once

#include <QtCore/QHash>

#include "ReferenceMapper.h"

namespace AxiomModel {

    class CloneReferenceMapper : public ReferenceMapper {
    public:
        QUuid mapUuid(const QUuid &input) override;

        QPoint mapPos(const QUuid &parent, const QPoint &input) override;

        void setUuid(const QUuid &key, const QUuid &value);

        void setPos(const QUuid &key, const QPoint &value);

    private:
        QHash<QUuid, QPoint> _pos;
        QHash<QUuid, QUuid> _values;
    };

}
