#pragma once

#include <QtCore/QHash>

#include "ReferenceMapper.h"

namespace AxiomModel {

    class CloneReferenceMapper : public ReferenceMapper {
    public:
        QUuid map(const QUuid &input) override;

        void set(const QUuid &key, const QUuid &value);

    private:
        QHash<QUuid, QUuid> _values;
    };

}
