#pragma once

#include "Control.h"

namespace AxiomModel {

    class ScopeControl : public Control {
    public:
        ScopeControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                     bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid, ModelRoot *root);

        static std::unique_ptr<ScopeControl> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                    bool selected, QString name, bool showName,
                                                    const QUuid &exposerUuid, const QUuid &exposingUuid,
                                                    ModelRoot *root);

        static std::unique_ptr<ScopeControl> deserialize(QDataStream &stream, const QUuid &uuid,
                                                         const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                                                         QString name, bool showName, const QUuid &exposerUuid,
                                                         const QUuid &exposingUuid, ReferenceMapper *ref,
                                                         ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        void doRuntimeUpdate() override {}
    };
}
