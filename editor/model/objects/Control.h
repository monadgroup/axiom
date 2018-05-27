#pragma once

#include <optional>

#include "common/Event.h"
#include "common/Promise.h"
#include "../ModelObject.h"
#include "../grid/GridItem.h"
#include "../WatchSequence.h"
#include "../ConnectionWire.h"
#include "compiler/common/ControlType.h"

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
            MIDI_PORTAL,
            SCOPE
        };

        AxiomCommon::Event<const QString &> nameChanged;
        AxiomCommon::Event<bool> showNameChanged;
        AxiomCommon::Event<QPointF> worldPosChanged;
        AxiomCommon::Event<bool> isActiveChanged;
        AxiomCommon::Event<QUuid> exposerUuidChanged;
        AxiomCommon::Event<MaximRuntime::Control*> runtimeAttached;
        AxiomCommon::Event<> runtimeAboutToDetach;

        Control(ControlType controlType, ConnectionWire::WireType wireType, QUuid uuid, const QUuid &parentUuid,
                QPoint pos, QSize size, bool selected, QString name, bool showName, const QUuid &exposerUuid,
                const QUuid &exposingUuid, ModelRoot *root);

        static ControlType fromRuntimeType(MaximCommon::ControlType type);

        static std::unique_ptr<Control> createDefault(ControlType type, const QUuid &uuid, const QUuid &parentUuid, const QString &name, const QUuid &exposingUuid, ModelRoot *root);

        static std::unique_ptr<Control>
        deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, ReferenceMapper *ref, ModelRoot *root);

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

        bool showName() const { return _showName; }

        void setShowName(bool showName);

        QUuid exposerUuid() const { return _exposerUuid; }

        void setExposerUuid(QUuid exposerUuid);

        QUuid exposingUuid() const { return _exposingUuid; }

        bool isActive() const { return _isActive; }

        void setIsActive(bool isActive);

        WatchSequence<Connection *> &connections() { return _connections; }

        const WatchSequence<Connection *> &connections() const { return _connections; }

        WatchSequence<QUuid> &connectedControls() { return _connectedControls; }

        const WatchSequence<QUuid> &connectedControls() const { return _connectedControls; }

        QPointF worldPos() const;

        std::optional<MaximRuntime::Control *> runtime() const { return _runtime; }

        bool canAttachRuntime(MaximRuntime::Control *runtime);

        void attachRuntime(MaximRuntime::Control *runtime);

        void detachRuntime();

        virtual void doRuntimeUpdate() = 0;

        virtual void saveValue() = 0;

        virtual void restoreValue() = 0;

        Sequence<ModelObject*> links() override;

        void remove() override;

    private:
        ControlSurface *_surface;
        ControlType _controlType;
        ConnectionWire::WireType _wireType;
        QString _name;
        bool _showName = true;
        QUuid _exposerUuid;
        QUuid _exposingUuid;
        bool _isActive = false;

        std::optional<MaximRuntime::Control *> _runtime;

        WatchSequence<Connection *> _connections;
        WatchSequence<QUuid> _connectedControls;

        void updateSinkPos();

        void updateExposerRemoved();

        void updateExposerRuntime();
    };

}
