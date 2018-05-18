#include "CreateControlAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/NumControl.h"
#include "../objects/MidiControl.h"
#include "../objects/ExtractControl.h"
#include "../../util.h"

using namespace AxiomModel;

CreateControlAction::CreateControlAction(const QUuid &uuid, const QUuid &parentUuid, Control::ControlType type,
                                         QString name, AxiomModel::ModelRoot *root)
    : Action(ActionType::CREATE_CONTROL, root), uuid(uuid), parentUuid(parentUuid), type(type), name(std::move(name)) {
}

std::unique_ptr<CreateControlAction> CreateControlAction::create(const QUuid &uuid, const QUuid &parentUuid,
                                                                 Control::ControlType type, QString name,
                                                                 AxiomModel::ModelRoot *root) {
    return std::make_unique<CreateControlAction>(uuid, parentUuid, type, std::move(name), root);
}

std::unique_ptr<CreateControlAction> CreateControlAction::create(const QUuid &parentUuid, Control::ControlType type,
                                                                 QString name, AxiomModel::ModelRoot *root) {
    return create(QUuid::createUuid(), parentUuid, type, std::move(name), root);
}

std::unique_ptr<CreateControlAction> CreateControlAction::deserialize(QDataStream &stream,
                                                                            AxiomModel::ModelRoot *root) {
    QUuid uuid; stream >> uuid;
    QUuid parentUuid; stream >> parentUuid;
    uint8_t typeInt; stream >> typeInt;
    QString name; stream >> name;

    return create(uuid, parentUuid, (Control::ControlType) typeInt, std::move(name), root);
}

void CreateControlAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << uuid;
    stream << parentUuid;
    stream << (uint8_t) type;
    stream << name;
}

bool CreateControlAction::forward(bool) {
    switch (type) {
        case Control::ControlType::NUM_SCALAR:
            root()->pool().registerObj(NumControl::create(uuid, parentUuid, QPoint(0, 0), QSize(2, 2), false, name, true, NumControl::DisplayMode::KNOB, NumControl::Channel::BOTH, MaximRuntime::NumValue(), root()));
            break;
        case Control::ControlType::MIDI_SCALAR:
            root()->pool().registerObj(MidiControl::create(uuid, parentUuid, QPoint(0, 0), QSize(2, 2), false, name, true, root()));
            break;
        case Control::ControlType::NUM_EXTRACT:
            root()->pool().registerObj(ExtractControl::create(uuid, parentUuid, QPoint(0, 0), QSize(2, 2), false, name, true, ConnectionWire::WireType::NUM, 0, root()));
            break;
        case Control::ControlType::MIDI_EXTRACT:
            root()->pool().registerObj(ExtractControl::create(uuid, parentUuid, QPoint(0, 0), QSize(2, 2), false, name, true, ConnectionWire::WireType::MIDI, 0, root()));
            break;
        default: unreachable;
    }
    return false;
}

bool CreateControlAction::backward() {
    find(root()->controls(), uuid)->remove();
    return false;
}
