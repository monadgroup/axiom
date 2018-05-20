#include "Control.h"

#include "ControlSurface.h"
#include "Connection.h"
#include "GroupNode.h"
#include "NumControl.h"
#include "MidiControl.h"
#include "ExtractControl.h"
#include "PortalControl.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "compiler/codegen/Control.h"
#include "compiler/runtime/Control.h"
#include "compiler/runtime/GroupNode.h"

using namespace AxiomModel;

Control::Control(AxiomModel::Control::ControlType controlType, AxiomModel::ConnectionWire::WireType wireType,
                 QUuid uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                 bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid, AxiomModel::ModelRoot *root)
    : GridItem(&find(root->controlSurfaces(), parentUuid)->grid(), pos, size, selected),
      ModelObject(ModelType::CONTROL, uuid, parentUuid, root), _surface(find(root->controlSurfaces(), parentUuid)),
      _controlType(controlType), _wireType(wireType), _name(std::move(name)), _showName(showName),
      _exposerUuid(exposerUuid), _exposingUuid(exposingUuid),
      _connections(filterWatch(root->connections(), std::function([uuid](Connection *const &connection) {
          return connection->controlA()->uuid() == uuid || connection->controlB()->uuid() == uuid;
      }))), _connectedControls(
        mapFilterWatch(_connections, std::function([uuid](Connection *const &connection) -> std::optional<Control *> {
            if (connection->controlA()->uuid() == uuid) return connection->controlB();
            if (connection->controlB()->uuid() == uuid) return connection->controlA();
            return std::optional<Control *>();
        }))) {
    posChanged.connect(this, &Control::updateSinkPos);
    removed.connect(this, &Control::updateExposerRemoved);
    _surface->node()->posChanged.connect(this, &Control::updateSinkPos);
}

Control::ControlType Control::fromRuntimeType(MaximCommon::ControlType type) {
    switch (type) {
        case MaximCommon::ControlType::NUMBER: return Control::ControlType::NUM_SCALAR;
        case MaximCommon::ControlType::MIDI: return Control::ControlType::MIDI_SCALAR;
        case MaximCommon::ControlType::NUM_EXTRACT: return Control::ControlType::NUM_EXTRACT;
        case MaximCommon::ControlType::MIDI_EXTRACT: return Control::ControlType::MIDI_EXTRACT;
        default: unreachable;
    }
}

std::unique_ptr<Control> Control::createDefault(AxiomModel::Control::ControlType type, const QUuid &uuid,
                                                const QUuid &parentUuid, const QString &name,
                                                const QUuid &exposingUuid, AxiomModel::ModelRoot *root) {
    switch (type) {
        case Control::ControlType::NUM_SCALAR:
            return NumControl::create(uuid, parentUuid, QPoint(0, 0), QSize(2, 2), false, name, true, QUuid(), exposingUuid, NumControl::DisplayMode::KNOB, NumControl::Channel::BOTH, MaximRuntime::NumValue(), root);
        case Control::ControlType::MIDI_SCALAR:
            return MidiControl::create(uuid, parentUuid, QPoint(0, 0), QSize(2, 2), false, name, true, QUuid(), exposingUuid, root);
        case Control::ControlType::NUM_EXTRACT:
            return ExtractControl::create(uuid, parentUuid, QPoint(0, 0), QSize(2, 2), false, name, true, QUuid(), exposingUuid, ConnectionWire::WireType::NUM, 0, root);
        case Control::ControlType::MIDI_EXTRACT:
            return ExtractControl::create(uuid, parentUuid, QPoint(0, 0), QSize(2, 2), false, name, true, QUuid(), exposingUuid, ConnectionWire::WireType::MIDI, 0, root);
        default: unreachable;
    }
}

std::unique_ptr<Control> Control::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                              AxiomModel::ModelRoot *root) {
    uint8_t controlTypeInt;
    stream >> controlTypeInt;

    QPoint pos;
    QSize size;
    bool selected;
    GridItem::deserialize(stream, pos, size, selected);

    QString name;
    stream >> name;

    bool showName;
    stream >> showName;

    QUuid exposerUuid;
    stream >> exposerUuid;

    QUuid exposingUuid;
    stream >> exposingUuid;

    switch ((ControlType) controlTypeInt) {
        case ControlType::NUM_SCALAR:
            return NumControl::deserialize(stream, uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid, exposingUuid, root);
        case ControlType::MIDI_SCALAR:
            return MidiControl::deserialize(stream, uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid, exposingUuid, root);
        case ControlType::NUM_EXTRACT:
            return ExtractControl::deserialize(stream, uuid, parentUuid, pos, size, selected, std::move(name),
                                               showName, exposerUuid, exposingUuid, ConnectionWire::WireType::NUM, root);
        case ControlType::MIDI_EXTRACT:
            return ExtractControl::deserialize(stream, uuid, parentUuid, pos, size, selected, std::move(name),
                                               showName, exposerUuid, exposingUuid, ConnectionWire::WireType::MIDI, root);
        case ControlType::NUM_PORTAL:
            return PortalControl::deserialize(stream, uuid, parentUuid, pos, size, selected, std::move(name), showName,
                                              exposerUuid, exposingUuid, ConnectionWire::WireType::NUM, root);
        case ControlType::MIDI_PORTAL:
            return PortalControl::deserialize(stream, uuid, parentUuid, pos, size, selected, std::move(name), showName,
                                              exposerUuid, exposingUuid, ConnectionWire::WireType::MIDI, root);
    }

    unreachable;
}

void Control::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    ModelObject::serialize(stream, parent, withContext);

    stream << (uint8_t) _controlType;
    GridItem::serialize(stream);
    stream << _name;
    stream << _showName;
    stream << _exposerUuid;
    stream << _exposingUuid;
}

void Control::setName(const QString &name) {
    if (name != _name) {
        _name = name;
        nameChanged.trigger(name);
    }
}

void Control::setShowName(bool showName) {
    if (showName != _showName) {
        _showName = showName;
        showNameChanged.trigger(showName);
    }
}

void Control::setExposerUuid(QUuid exposerUuid) {
    if (exposerUuid != _exposerUuid) {
        _exposerUuid = exposerUuid;
        exposerUuidChanged.trigger(exposerUuid);

        updateExposerRuntime();
    }
}

void Control::setIsActive(bool isActive) {
    if (isActive != _isActive) {
        _isActive = isActive;
        isActiveChanged.trigger(isActive);
    }
}

QPointF Control::worldPos() const {
    auto worldPos = pos() + ControlSurface::nodeToControl(_surface->node()->pos());
    auto centerPos = worldPos + QPointF(size().width() / 2., size().height() / 2.);
    return ControlSurface::controlToNode(centerPos);
}

bool Control::canAttachRuntime(MaximRuntime::Control *runtime) {
    return fromRuntimeType(runtime->type()->type()) == controlType() && name() == QString::fromStdString(runtime->name()) && !_runtime;
}

void Control::attachRuntime(MaximRuntime::Control *runtime) {
    assert(!_runtime);

    _runtime = runtime;
    runtime->removed.connect(this, &Control::detachRuntime);

    runtimeAttached.trigger(runtime);
    updateExposerRuntime();
}

void Control::detachRuntime() {
    runtimeAboutToDetach.trigger();
    _runtime.reset();
}

Sequence<ModelObject*> Control::links() {
    auto expId = exposerUuid();
    return flatten(std::array<Sequence<ModelObject*>, 2> {
        staticCast<ModelObject*>(filter(root()->controls(), std::function([expId](Control *const &obj) -> bool { return obj->uuid() == expId; }))).sequence(),
        staticCast<ModelObject*>(_connections.sequence()).sequence()
    });
}

void Control::remove() {
    ModelObject::remove();
}

void Control::updateSinkPos() {
    worldPosChanged.trigger(worldPos());
}

void Control::updateExposerRuntime() {
    //if (_runtime && !_exposerUuid.isNull()) {
    //    std::cout << "Attaching runtime when control becomes available..." << std::endl;
    //    findLater<Control*>(root()->controls(), _exposerUuid).then([this](Control *const &control) {
    //        auto controlNode = dynamic_cast<GroupNode*>(control->surface()->node());
    //        assert(controlNode);
    //        assert(controlNode->runtime());

    //        std::cout << "Attaching runtime!" << std::endl;
    //        auto newRuntime = (*controlNode->runtime())->forwardControl(*_runtime);
    //        control->attachRuntime(newRuntime);
    //        control->removed.connect([newRuntime]() { newRuntime->remove(); });
    //        std::cout << "Finished attaching runtime" << std::endl;
    //    });
    //}
}

void Control::updateExposerRemoved() {
    //if (!_exposingUuid.isNull()) find(root()->controls(), _exposingUuid)->setExposerUuid(QUuid());
}
