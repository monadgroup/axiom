#pragma once

#include "../ModelObject.h"
#include "../grid/GridItem.h"

namespace AxiomModel {

    class Control : public GridItem, public ModelObject {
    public:
        enum class ControlType {
            NUM_SCALAR,
            MIDI_SCALAR
        };

        enum class ValueType {
            NUM,
            MIDI
        };

        Event<const QString &> nameChanged;

        Control(ControlType controlType, ValueType valueType, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, ModelRoot *root);

        static std::unique_ptr<Control> deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        ControlType controlType() const { return _controlType; }

        ValueType valueType() const { return _valueType; }

        bool isMovable() const override { return true; }

        bool isResizable() const override { return true; }

        bool isCopyable() const override { return false; }

        bool isDeletable() const override { return false; }

        const QString &name() const { return _name; }

        void setName(const QString &name);

        void remove() override;

    private:
        ControlType _controlType;
        ValueType _valueType;
        QString _name;
    };

}
