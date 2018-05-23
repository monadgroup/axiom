#pragma once

#include "common/Event.h"
#include "Control.h"
#include "compiler/runtime/ValueOperator.h"

namespace AxiomModel {

    class MidiControl : public Control {
    public:
        AxiomCommon::Event<const MaximRuntime::MidiValue &> valueChanged;

        MidiControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                    bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid, ModelRoot *root);

        static std::unique_ptr<MidiControl> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                   bool selected, QString name, bool showName,
                                                   const QUuid &exposerUuid, const QUuid &exposingUuid, ModelRoot *root);

        static std::unique_ptr<MidiControl>
        deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                    bool selected, QString name, bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                    ReferenceMapper *ref, ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        const MaximRuntime::MidiValue &value() const { return _value; }

        void setValue(const MaximRuntime::MidiValue &value);

        void doRuntimeUpdate() override;

        void saveValue() override;

        void restoreValue() override;

    private:
        MaximRuntime::MidiValue _value;

        void setInternalValue(MaximRuntime::MidiValue value);
    };

}
