#pragma once

#include "common/Event.h"
#include "Control.h"
#include "../Value.h"

namespace AxiomModel {

    class MidiControl : public Control {
    public:
        AxiomCommon::Event<const MidiValue &> valueChanged;

        MidiControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                    bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid, ModelRoot *root);

        static std::unique_ptr<MidiControl> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                   bool selected, QString name, bool showName,
                                                   const QUuid &exposerUuid, const QUuid &exposingUuid,
                                                   ModelRoot *root);

        static std::unique_ptr<MidiControl>
        deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                    bool selected, QString name, bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                    ReferenceMapper *ref, ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        const MidiValue &value() const { return _value; }

        void setValue(const MidiValue &value);

        void saveValue() override;

        void restoreValue() override;

    private:
        MidiValue _value;

        void setInternalValue(MidiValue value);
    };

}
