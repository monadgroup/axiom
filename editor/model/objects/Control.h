#pragma once

#include <optional>

#include "common/Event.h"
#include "../ModelObject.h"
#include "../grid/GridItem.h"
#include "../WatchSequence.h"
#include "../ConnectionWire.h"

namespace MaximRuntime {
    class Control;
}

namespace AxiomModel {

    class ControlSurface;

    class Connection;

    class Control : public GridItem, public ModelObject {
    public:
        enum class ControlType {
            NUM_SCALAR,
            MIDI_SCALAR,
            NUM_EXTRACT,
            MIDI_EXTRACT,
            NUM_PORTAL,
            MIDI_PORTAL
        };

        AxiomCommon::Event<const QString &> nameChanged;
        AxiomCommon::Event<QPointF> worldPosChanged;
        AxiomCommon::Event<bool> isActiveChanged;

        Control(ControlType controlType, ConnectionWire::WireType wireType, QUuid uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, ModelRoot *root);

        static std::unique_ptr<Control> deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        ControlSurface *surface() const { return _surface; }

        ControlType controlType() const { return _controlType; }

        ConnectionWire::WireType wireType() const { return _wireType; }

        bool isMovable() const override { return true; }

        bool isResizable() const override { return true; }

        bool isCopyable() const override { return false; }

        bool isDeletable() const override { return false; }

        const QString &name() const { return _name; }

        void setName(const QString &name);

        bool isActive() const { return _isActive; }

        void setIsActive(bool isActive);

        WatchSequence<Connection *> &connections() { return _connections; }

        const WatchSequence<Connection *> &connections() const { return _connections; }

        WatchSequence<Control *> &connectedControls() { return _connectedControls; }

        const WatchSequence<Control *> &connectedControls() const { return _connectedControls; }

        QPointF worldPos() const;

        void attachRuntime(MaximRuntime::Control *runtime);

        void detachRuntime();

        void remove() override;

    private:
        ControlSurface *_surface;
        ControlType _controlType;
        ConnectionWire::WireType _wireType;
        QString _name;
        bool _isActive = false;

        std::optional<MaximRuntime::Control *> _runtime;

        WatchSequence<Connection *> _connections;
        WatchSequence<Control *> _connectedControls;

        void updateSinkPos();
    };

}
