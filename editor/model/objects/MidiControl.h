#pragma once

#include "../Value.h"
#include "Control.h"
#include "common/Event.h"

namespace AxiomModel {

    class MidiControl : public Control {
    public:
        AxiomCommon::Event<const MidiValue &> valueChanged;

        MidiControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                    bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid, ModelRoot *root);

        static std::unique_ptr<MidiControl> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                   bool selected, QString name, bool showName, const QUuid &exposerUuid,
                                                   const QUuid &exposingUuid, ModelRoot *root);

        static std::unique_ptr<MidiControl> deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                        QPoint pos, QSize size, bool selected, QString name,
                                                        bool showName, const QUuid &exposerUuid,
                                                        const QUuid &exposingUuid, ReferenceMapper *ref,
                                                        ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        void doRuntimeUpdate() override {}
    };
}
