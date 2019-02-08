#include <utility>

#pragma once

#include <optional>

#include "../ConnectionWire.h"
#include "../ModelObject.h"
#include "../grid/GridItem.h"
#include "common/Event.h"
#include "common/Promise.h"
#include "common/WatchSequence.h"
#include "editor/compiler/interface/ControlInitializer.h"
#include "editor/compiler/interface/Frontend.h"

namespace MaximCompiler {
    class Runtime;
}

namespace AxiomModel {

    class ControlSurface;

    class Connection;

    class CompositeAction;

    struct ControlCompileMeta {
        size_t index;
        bool writtenTo;
        bool readFrom;

        ControlCompileMeta(size_t index, bool writtenTo, bool readFrom)
            : index(index), writtenTo(writtenTo), readFrom(readFrom) {}
    };

    struct ControlPrepare {
        QPoint pos;
        QSize size;
        std::unique_ptr<CompositeAction> preActions;
    };

    class Control : public GridItem, public ModelObject {
    public:
        enum class ControlType { NUM_SCALAR, MIDI_SCALAR, NUM_EXTRACT, MIDI_EXTRACT, NUM_PORTAL, MIDI_PORTAL, GRAPH };

        AxiomCommon::Event<const QString &> nameChanged;
        AxiomCommon::Event<bool> showNameChanged;
        AxiomCommon::Event<QPointF> worldPosChanged;
        AxiomCommon::Event<bool> isActiveChanged;
        AxiomCommon::Event<QUuid> exposerUuidChanged;
        AxiomCommon::Event<bool> isEnabledChanged;

        Control(ControlType controlType, ConnectionWire::WireType wireType, QSize minSize, QUuid uuid,
                const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, bool showName,
                const QUuid &exposerUuid, const QUuid &exposingUuid, ModelRoot *root);

        static QSize getDefaultSize(ControlType controlType);

        static std::unique_ptr<Control> createDefault(ControlType type, const QUuid &uuid, const QUuid &parentUuid,
                                                      const QString &name, const QUuid &exposingUuid, QPoint pos,
                                                      QSize size, bool isWrittenTo, ModelRoot *root);

        static std::unique_ptr<Control> createExposed(Control *base, const QUuid &uuid, const QUuid &parentUuid,
                                                      QPoint pos, QSize size);

        static ControlPrepare buildControlPrepareAction(ControlType type, const QUuid &parentUuid, ModelRoot *root);

        ControlSurface *surface() const { return _surface; }

        ControlType controlType() const { return _controlType; }

        ConnectionWire::WireType wireType() const { return _wireType; }

        bool isEnabled() const;

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

        AxiomCommon::BoxedWatchSequence<Connection *> &connections() { return _connections; }

        const AxiomCommon::BoxedWatchSequence<Connection *> &connections() const { return _connections; }

        AxiomCommon::BoxedWatchSequence<QUuid> &connectedControls() { return _connectedControls; }

        const AxiomCommon::BoxedWatchSequence<QUuid> &connectedControls() const { return _connectedControls; }

        QPointF worldPos() const;

        AxiomCommon::BoxedSequence<ModelObject *> links() override;

        virtual MaximCompiler::ControlInitializer getInitializer();

        const std::optional<ControlCompileMeta> &compileMeta() const;

        const std::optional<MaximFrontend::ControlPointers> &runtimePointers() const;

        void setCompileMeta(std::optional<ControlCompileMeta> compileMeta) { _compileMeta = std::move(compileMeta); }

        void setRuntimePointers(std::optional<MaximFrontend::ControlPointers> runtimePointers) {
            _runtimePointers = std::move(runtimePointers);

            restoreState();
        }

    private:
        ControlSurface *_surface;
        ControlType _controlType;
        ConnectionWire::WireType _wireType;
        QString _name;
        bool _showName = true;
        QUuid _exposerUuid;
        QUuid _exposingUuid;
        bool _isActive = false;
        std::optional<ControlCompileMeta> _compileMeta;
        std::optional<MaximFrontend::ControlPointers> _runtimePointers;

        AxiomCommon::BoxedWatchSequence<Connection *> _connections;
        AxiomCommon::BoxedWatchSequence<QUuid> _connectedControls;

        void updateSinkPos();

        void updateExposerRemoved();

        void updateExposingName(Control *exposingControl);
    };
}
