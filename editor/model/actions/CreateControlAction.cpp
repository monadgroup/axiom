#include "CreateControlAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/NumControl.h"
#include "../objects/MidiControl.h"
#include "../objects/ExtractControl.h"

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
    QUuid uuid;
    stream >> uuid;
    QUuid parentUuid;
    stream >> parentUuid;
    uint8_t typeInt;
    stream >> typeInt;
    QString name;
    stream >> name;

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
    root()->pool().registerObj(Control::createDefault(type, uuid, parentUuid, name, QUuid(), root()));
    return false;
}

bool CreateControlAction::backward() {
    find(root()->controls(), uuid)->remove();
    return false;
}
