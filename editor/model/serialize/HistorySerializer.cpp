#include "HistorySerializer.h"

#include "../../util.h"
#include "../HistoryList.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../actions/Action.h"
#include "../actions/AddGraphPointAction.h"
#include "../actions/CompositeAction.h"
#include "../actions/CreateConnectionAction.h"
#include "../actions/CreateControlAction.h"
#include "../actions/CreateCustomNodeAction.h"
#include "../actions/CreateGroupNodeAction.h"
#include "../actions/CreatePortalNodeAction.h"
#include "../actions/DeleteGraphPointAction.h"
#include "../actions/DeleteObjectAction.h"
#include "../actions/ExposeControlAction.h"
#include "../actions/GridItemMoveAction.h"
#include "../actions/GridItemSizeAction.h"
#include "../actions/MoveGraphPointAction.h"
#include "../actions/PasteBufferAction.h"
#include "../actions/RenameControlAction.h"
#include "../actions/RenameNodeAction.h"
#include "../actions/SetCodeAction.h"
#include "../actions/SetGraphTagAction.h"
#include "../actions/SetGraphTensionAction.h"
#include "../actions/SetNumModeAction.h"
#include "../actions/SetNumRangeAction.h"
#include "../actions/SetNumValueAction.h"
#include "../actions/SetShowNameAction.h"
#include "../actions/UnexposeControlAction.h"
#include "../objects/RootSurface.h"
#include "ValueSerializer.h"

using namespace AxiomModel;

void HistorySerializer::serialize(const AxiomModel::HistoryList &history, QDataStream &stream) {
    stream << (uint32_t) history.stackPos();
    stream << (uint32_t) history.stack().size();
    for (const auto &action : history.stack()) {
        QByteArray actionBuffer;
        QDataStream actionStream(&actionBuffer, QIODevice::WriteOnly);
        serializeAction(action.get(), actionStream);
        stream << actionBuffer;
    }
}

HistoryList HistorySerializer::deserialize(QDataStream &stream, uint32_t version, ModelRoot *root,
                                           HistoryList::CompileApplyer applyer) {
    uint32_t stackPos;
    stream >> stackPos;
    uint32_t stackSize;
    stream >> stackSize;

    std::vector<std::unique_ptr<Action>> stack;
    stack.reserve(stackSize);
    for (uint32_t i = 0; i < stackSize; i++) {
        QByteArray actionBuffer;
        stream >> actionBuffer;
        QDataStream actionStream(&actionBuffer, QIODevice::ReadOnly);
        stack.push_back(deserializeAction(actionStream, version, root));
    }

    return HistoryList(stackPos, std::move(stack), std::move(applyer));
}

void HistorySerializer::serializeAction(AxiomModel::Action *action, QDataStream &stream) {
    stream << (uint8_t) action->actionType();

    if (auto composite = dynamic_cast<CompositeAction *>(action))
        serializeCompositeAction(composite, stream);
    else if (auto deleteObject = dynamic_cast<DeleteObjectAction *>(action))
        serializeDeleteObjectAction(deleteObject, stream);
    else if (auto createCustomNode = dynamic_cast<CreateCustomNodeAction *>(action))
        serializeCreateCustomNodeAction(createCustomNode, stream);
    else if (auto createGroupNode = dynamic_cast<CreateGroupNodeAction *>(action))
        serializeCreateGroupNodeAction(createGroupNode, stream);
    else if (auto createPortalNode = dynamic_cast<CreatePortalNodeAction *>(action))
        serializeCreatePortalNodeAction(createPortalNode, stream);
    else if (auto createConnection = dynamic_cast<CreateConnectionAction *>(action))
        serializeCreateConnectionAction(createConnection, stream);
    else if (auto gridItemMove = dynamic_cast<GridItemMoveAction *>(action))
        serializeGridItemMoveAction(gridItemMove, stream);
    else if (auto gridItemSize = dynamic_cast<GridItemSizeAction *>(action))
        serializeGridItemSizeAction(gridItemSize, stream);
    else if (auto renameControl = dynamic_cast<RenameControlAction *>(action))
        serializeRenameControlAction(renameControl, stream);
    else if (auto renameNode = dynamic_cast<RenameNodeAction *>(action))
        serializeRenameNodeAction(renameNode, stream);
    else if (auto setCode = dynamic_cast<SetCodeAction *>(action))
        serializeSetCodeAction(setCode, stream);
    else if (auto createControl = dynamic_cast<CreateControlAction *>(action))
        serializeCreateControlAction(createControl, stream);
    else if (auto setNumMode = dynamic_cast<SetNumModeAction *>(action))
        serializeSetNumModeAction(setNumMode, stream);
    else if (auto setNumValue = dynamic_cast<SetNumValueAction *>(action))
        serializeSetNumValueAction(setNumValue, stream);
    else if (auto setShowName = dynamic_cast<SetShowNameAction *>(action))
        serializeSetShowNameAction(setShowName, stream);
    else if (auto exposeControl = dynamic_cast<ExposeControlAction *>(action))
        serializeExposeControlAction(exposeControl, stream);
    else if (auto pasteBuffer = dynamic_cast<PasteBufferAction *>(action))
        serializePasteBufferAction(pasteBuffer, stream);
    else if (auto unexposeControl = dynamic_cast<UnexposeControlAction *>(action))
        serializeUnexposeControlAction(unexposeControl, stream);
    else if (auto addGraphPoint = dynamic_cast<AddGraphPointAction *>(action))
        serializeAddGraphPointAction(addGraphPoint, stream);
    else if (auto deleteGraphPoint = dynamic_cast<DeleteGraphPointAction *>(action))
        serializeDeleteGraphPointAction(deleteGraphPoint, stream);
    else if (auto moveGraphPoint = dynamic_cast<MoveGraphPointAction *>(action))
        serializeMoveGraphPointAction(moveGraphPoint, stream);
    else if (auto setGraphTag = dynamic_cast<SetGraphTagAction *>(action))
        serializeSetGraphTagAction(setGraphTag, stream);
    else if (auto setGraphTension = dynamic_cast<SetGraphTensionAction *>(action))
        serializeSetGraphTensionAction(setGraphTension, stream);
    else if (auto setNumRange = dynamic_cast<SetNumRangeAction *>(action))
        serializeSetNumRangeAction(setNumRange, stream);
    else
        unreachable;
}

std::unique_ptr<Action> HistorySerializer::deserializeAction(QDataStream &stream, uint32_t version,
                                                             AxiomModel::ModelRoot *root) {
    uint8_t actionTypeInt;
    stream >> actionTypeInt;

    switch ((Action::ActionType) actionTypeInt) {
    case Action::ActionType::NONE:
        unreachable;
    case Action::ActionType::COMPOSITE:
        return deserializeCompositeAction(stream, version, root);
    case Action::ActionType::DELETE_OBJECT:
        return deserializeDeleteObjectAction(stream, version, root);
    case Action::ActionType::CREATE_CUSTOM_NODE:
        return deserializeCreateCustomNodeAction(stream, version, root);
    case Action::ActionType::CREATE_GROUP_NODE:
        return deserializeCreateGroupNodeAction(stream, version, root);
    case Action::ActionType::CREATE_PORTAL_NODE:
        return deserializeCreatePortalNodeAction(stream, version, root);
    case Action::ActionType::CREATE_CONNECTION:
        return deserializeCreateConnectionAction(stream, version, root);
    case Action::ActionType::MOVE_GRID_ITEM:
        return deserializeGridItemMoveAction(stream, version, root);
    case Action::ActionType::SIZE_GRID_ITEM:
        return deserializeGridItemSizeAction(stream, version, root);
    case Action::ActionType::RENAME_CONTROL:
        return deserializeRenameControlAction(stream, version, root);
    case Action::ActionType::RENAME_NODE:
        return deserializeRenameNodeAction(stream, version, root);
    case Action::ActionType::SET_CODE:
        return deserializeSetCodeAction(stream, version, root);
    case Action::ActionType::CREATE_CONTROL:
        return deserializeCreateControlAction(stream, version, root);
    case Action::ActionType::SET_NUM_MODE:
        return deserializeSetNumModeAction(stream, version, root);
    case Action::ActionType::SET_NUM_VALUE:
        return deserializeSetNumValueAction(stream, version, root);
    case Action::ActionType::SET_SHOW_NAME:
        return deserializeSetShowNameAction(stream, version, root);
    case Action::ActionType::EXPOSE_CONTROL:
        return deserializeExposeControlAction(stream, version, root);
    case Action::ActionType::PASTE_BUFFER:
        return deserializePasteBufferAction(stream, version, root);
    case Action::ActionType::UNEXPOSE_CONTROL:
        return deserializeUnexposeControlAction(stream, version, root);
    case Action::ActionType::ADD_GRAPH_POINT:
        return deserializeAddGraphPointAction(stream, version, root);
    case Action::ActionType::DELETE_GRAPH_POINT:
        return deserializeDeleteGraphPointAction(stream, version, root);
    case Action::ActionType::MOVE_GRAPH_POINT:
        return deserializeMoveGraphPointAction(stream, version, root);
    case Action::ActionType::SET_GRAPH_TAG:
        return deserializeSetGraphTagAction(stream, version, root);
    case Action::ActionType::SET_GRAPH_TENSION:
        return deserializeSetGraphTensionAction(stream, version, root);
    case Action::ActionType::SET_NUM_RANGE:
        return deserializeSetNumRangeAction(stream, version, root);
    }

    unreachable;
}

void HistorySerializer::serializeCompositeAction(AxiomModel::CompositeAction *action, QDataStream &stream) {
    stream << (uint32_t) action->actions().size();
    for (const auto &subAction : action->actions()) {
        serializeAction(subAction.get(), stream);
    }
}

std::unique_ptr<Action> HistorySerializer::deserializeCompositeAction(QDataStream &stream, uint32_t version,
                                                                      AxiomModel::ModelRoot *root) {
    uint32_t actionCount;
    stream >> actionCount;

    std::vector<std::unique_ptr<Action>> actions;
    actions.reserve(actionCount);

    for (uint32_t i = 0; i < actionCount; i++) {
        actions.push_back(deserializeAction(stream, version, root));
    }

    // if the first action is SetCode, migrate this to the new set code format
    if (version < 3 && !actions.empty() && dynamic_cast<SetCodeAction *>(actions[0].get())) {
        std::cout << "Performing migration: CompositeAction containing SetCodeAction being converted to SetCodeAction ("
                  << actions.size() - 1 << " action/s)" << std::endl;
        std::unique_ptr<SetCodeAction> setCodeAction((SetCodeAction *) std::move(actions[0]).release());
        assert(setCodeAction->controlActions().empty());
        setCodeAction->controlActions().reserve(actions.size() - 1);
        for (auto i = actions.begin() + 1; i < actions.end(); i++) {
            setCodeAction->controlActions().push_back(std::move(*i));
        }

        return setCodeAction;
    } else {
        return CompositeAction::create(std::move(actions), root);
    }
}

void HistorySerializer::serializeDeleteObjectAction(AxiomModel::DeleteObjectAction *action, QDataStream &stream) {
    stream << action->uuid();
    stream << action->buffer();
}

std::unique_ptr<DeleteObjectAction> HistorySerializer::deserializeDeleteObjectAction(QDataStream &stream,
                                                                                     uint32_t version,
                                                                                     AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QByteArray buffer;
    stream >> buffer;

    return DeleteObjectAction::create(uuid, std::move(buffer), root);
}

void HistorySerializer::serializeCreateCustomNodeAction(AxiomModel::CreateCustomNodeAction *action,
                                                        QDataStream &stream) {
    stream << action->uuid();
    stream << action->parentUuid();
    stream << action->pos();
    stream << action->name();
    stream << action->controlsUuid();
}

std::unique_ptr<CreateCustomNodeAction>
    HistorySerializer::deserializeCreateCustomNodeAction(QDataStream &stream, uint32_t version,
                                                         AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QUuid parentUuid;
    stream >> parentUuid;
    QPoint pos;
    stream >> pos;
    QString name;
    stream >> name;
    QUuid controlsUuid;
    stream >> controlsUuid;

    return CreateCustomNodeAction::create(uuid, parentUuid, pos, std::move(name), controlsUuid, root);
}

void HistorySerializer::serializeCreateGroupNodeAction(AxiomModel::CreateGroupNodeAction *action, QDataStream &stream) {
    stream << action->uuid();
    stream << action->parentUuid();
    stream << action->pos();
    stream << action->name();
    stream << action->controlsUuid();
    stream << action->innerUuid();
}

std::unique_ptr<CreateGroupNodeAction>
    HistorySerializer::deserializeCreateGroupNodeAction(QDataStream &stream, uint32_t version,
                                                        AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QUuid parentUuid;
    stream >> parentUuid;
    QPoint pos;
    stream >> pos;
    QString name;
    stream >> name;
    QUuid controlsUuid;
    stream >> controlsUuid;
    QUuid innerUuid;
    stream >> innerUuid;

    return CreateGroupNodeAction::create(uuid, parentUuid, pos, std::move(name), controlsUuid, innerUuid, root);
}

void HistorySerializer::serializeCreatePortalNodeAction(AxiomModel::CreatePortalNodeAction *action,
                                                        QDataStream &stream) {
    stream << action->uuid();
    stream << action->parentUuid();
    stream << action->pos();
    stream << action->name();
    stream << action->controlsUuid();
    stream << (uint8_t) action->wireType();
    stream << (uint8_t) action->portalType();
    stream << action->controlUuid();
    stream << (quint64) action->portalId();
}

std::unique_ptr<CreatePortalNodeAction>
    HistorySerializer::deserializeCreatePortalNodeAction(QDataStream &stream, uint32_t version,
                                                         AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QUuid parentUuid;
    stream >> parentUuid;
    QPoint pos;
    stream >> pos;
    QString name;
    stream >> name;
    QUuid controlsUuid;
    stream >> controlsUuid;
    uint8_t wireTypeInt;
    stream >> wireTypeInt;
    uint8_t portalTypeInt;
    stream >> portalTypeInt;
    QUuid controlUuid;
    stream >> controlUuid;

    // unique portal IDs were added in 0.4.0, corresponding to schema version 5
    quint64 nextPortalId;
    if (version >= 5) {
        stream >> nextPortalId;
    } else {
        nextPortalId =
            takeAt(dynamicCast<RootSurface *>(findChildren(root->nodeSurfaces(), QUuid())), 0)->takePortalId();
    }

    return CreatePortalNodeAction::create(uuid, parentUuid, pos, std::move(name), controlsUuid,
                                          (ConnectionWire::WireType) wireTypeInt,
                                          (PortalControl::PortalType) portalTypeInt, nextPortalId, controlUuid, root);
}

void HistorySerializer::serializeCreateConnectionAction(AxiomModel::CreateConnectionAction *action,
                                                        QDataStream &stream) {
    stream << action->uuid();
    stream << action->parentUuid();
    stream << action->controlA();
    stream << action->controlB();
}

std::unique_ptr<CreateConnectionAction>
    HistorySerializer::deserializeCreateConnectionAction(QDataStream &stream, uint32_t version,
                                                         AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QUuid parentUuid;
    stream >> parentUuid;
    QUuid controlA;
    stream >> controlA;
    QUuid controlB;
    stream >> controlB;

    return CreateConnectionAction::create(uuid, parentUuid, controlA, controlB, root);
}

void HistorySerializer::serializeGridItemMoveAction(AxiomModel::GridItemMoveAction *action, QDataStream &stream) {
    stream << action->uuid();
    stream << action->beforePos();
    stream << action->afterPos();
}

std::unique_ptr<GridItemMoveAction> HistorySerializer::deserializeGridItemMoveAction(QDataStream &stream,
                                                                                     uint32_t version,
                                                                                     AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QPoint beforePos;
    stream >> beforePos;
    QPoint afterPos;
    stream >> afterPos;

    return GridItemMoveAction::create(uuid, beforePos, afterPos, root);
}

void HistorySerializer::serializeGridItemSizeAction(AxiomModel::GridItemSizeAction *action, QDataStream &stream) {
    stream << action->uuid();
    stream << action->beforeRect();
    stream << action->afterRect();
}

std::unique_ptr<GridItemSizeAction> HistorySerializer::deserializeGridItemSizeAction(QDataStream &stream,
                                                                                     uint32_t version,
                                                                                     AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QRect beforeRect;
    stream >> beforeRect;
    QRect afterRect;
    stream >> afterRect;

    return GridItemSizeAction::create(uuid, beforeRect, afterRect, root);
}

void HistorySerializer::serializeRenameControlAction(AxiomModel::RenameControlAction *action, QDataStream &stream) {
    stream << action->uuid();
    stream << action->oldName();
    stream << action->newName();
}

std::unique_ptr<RenameControlAction> HistorySerializer::deserializeRenameControlAction(QDataStream &stream,
                                                                                       uint32_t version,
                                                                                       AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QString oldName;
    stream >> oldName;
    QString newName;
    stream >> newName;

    return RenameControlAction::create(uuid, std::move(oldName), std::move(newName), root);
}

void HistorySerializer::serializeRenameNodeAction(AxiomModel::RenameNodeAction *action, QDataStream &stream) {
    stream << action->uuid();
    stream << action->oldName();
    stream << action->newName();
}

std::unique_ptr<RenameNodeAction> HistorySerializer::deserializeRenameNodeAction(QDataStream &stream, uint32_t version,
                                                                                 AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QString oldName;
    stream >> oldName;
    QString newName;
    stream >> newName;

    return RenameNodeAction::create(uuid, std::move(oldName), std::move(newName), root);
}

void HistorySerializer::serializeSetCodeAction(AxiomModel::SetCodeAction *action, QDataStream &stream) {
    stream << action->uuid();
    stream << action->oldCode();
    stream << action->newCode();
    stream << (uint32_t) action->controlActions().size();
    for (const auto &subAction : action->controlActions()) {
        serializeAction(subAction.get(), stream);
    }
}

std::unique_ptr<SetCodeAction> HistorySerializer::deserializeSetCodeAction(QDataStream &stream, uint32_t version,
                                                                           AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QString oldCode;
    stream >> oldCode;
    QString newCode;
    stream >> newCode;

    std::vector<std::unique_ptr<Action>> controlActions;

    // Control actions being part of SetCode were only added in v3 - previously they were in a CompositeAction alongside
    // this one. CompositeAction has a migration to handle that, we just need to ignore the controls here.
    if (version >= 3) {
        uint32_t controlActionCount;
        stream >> controlActionCount;

        controlActions.reserve(controlActionCount);
        for (uint32_t i = 0; i < controlActionCount; i++) {
            controlActions.push_back(deserializeAction(stream, version, root));
        }
    }

    return SetCodeAction::create(uuid, std::move(oldCode), std::move(newCode), std::move(controlActions), root);
}

void HistorySerializer::serializeCreateControlAction(AxiomModel::CreateControlAction *action, QDataStream &stream) {
    stream << action->uuid();
    stream << action->parentUuid();
    stream << (uint8_t) action->type();
    stream << action->name();
    stream << action->pos();
    stream << action->size();
}

std::unique_ptr<CreateControlAction> HistorySerializer::deserializeCreateControlAction(QDataStream &stream,
                                                                                       uint32_t version,
                                                                                       AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QUuid parentUuid;
    stream >> parentUuid;
    uint8_t typeInt;
    stream >> typeInt;
    QString name;
    stream >> name;

    QPoint pos;
    QSize size;

    // Position/size was added in 0.4.0 (schema version 5) for the enhanced control placement feature. The previous
    // default was simply to place controls at (0, 0) with a size of (2, 2)
    if (version >= 5) {
        stream >> pos;
        stream >> size;
    } else {
        pos = QPoint(0, 0);
        size = QSize(2, 2);
    }

    return CreateControlAction::create(uuid, parentUuid, (Control::ControlType) typeInt, std::move(name), pos, size,
                                       root);
}

void HistorySerializer::serializeSetNumModeAction(AxiomModel::SetNumModeAction *action, QDataStream &stream) {
    stream << action->uuid();
    stream << (uint8_t) action->beforeMode();
    stream << (uint8_t) action->afterMode();
}

std::unique_ptr<SetNumModeAction> HistorySerializer::deserializeSetNumModeAction(QDataStream &stream, uint32_t version,
                                                                                 AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    uint8_t beforeModeInt;
    stream >> beforeModeInt;
    uint8_t afterModeInt;
    stream >> afterModeInt;

    return SetNumModeAction::create(uuid, (NumControl::DisplayMode) beforeModeInt,
                                    (NumControl::DisplayMode) afterModeInt, root);
}

void HistorySerializer::serializeSetNumValueAction(AxiomModel::SetNumValueAction *action, QDataStream &stream) {
    stream << action->uuid();
    ValueSerializer::serializeNum(action->beforeVal(), stream);
    ValueSerializer::serializeNum(action->afterVal(), stream);
}

std::unique_ptr<SetNumValueAction> HistorySerializer::deserializeSetNumValueAction(QDataStream &stream,
                                                                                   uint32_t version,
                                                                                   AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    auto beforeVal = ValueSerializer::deserializeNum(stream, version);
    auto afterVal = ValueSerializer::deserializeNum(stream, version);

    return SetNumValueAction::create(uuid, beforeVal, afterVal, root);
}

void HistorySerializer::serializeSetShowNameAction(AxiomModel::SetShowNameAction *action, QDataStream &stream) {
    stream << action->uuid();
    stream << action->beforeVal();
    stream << action->afterVal();
}

std::unique_ptr<SetShowNameAction> HistorySerializer::deserializeSetShowNameAction(QDataStream &stream,
                                                                                   uint32_t version,
                                                                                   AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    bool beforeVal;
    stream >> beforeVal;
    bool afterVal;
    stream >> afterVal;

    return SetShowNameAction::create(uuid, beforeVal, afterVal, root);
}

void HistorySerializer::serializeExposeControlAction(AxiomModel::ExposeControlAction *action, QDataStream &stream) {
    stream << action->controlUuid();
    stream << action->exposeUuid();
    stream << action->pos();
    stream << action->size();
}

std::unique_ptr<ExposeControlAction> HistorySerializer::deserializeExposeControlAction(QDataStream &stream,
                                                                                       uint32_t version,
                                                                                       AxiomModel::ModelRoot *root) {
    QUuid controlUuid;
    stream >> controlUuid;
    QUuid exposeUuid;
    stream >> exposeUuid;
    QPoint pos;
    stream >> pos;
    QSize size;
    stream >> size;

    return ExposeControlAction::create(controlUuid, exposeUuid, pos, size, root);
}

void HistorySerializer::serializePasteBufferAction(AxiomModel::PasteBufferAction *action, QDataStream &stream) {
    stream << action->surfaceUuid();
    stream << action->isBufferFormatted();
    stream << action->buffer();
    stream << action->usedUuids();
    stream << action->center();
}

std::unique_ptr<PasteBufferAction> HistorySerializer::deserializePasteBufferAction(QDataStream &stream,
                                                                                   uint32_t version,
                                                                                   AxiomModel::ModelRoot *root) {
    QUuid surfaceUuid;
    stream >> surfaceUuid;
    bool isBufferFormatted;
    stream >> isBufferFormatted;
    QByteArray buffer;
    stream >> buffer;
    QVector<QUuid> usedUuids;
    stream >> usedUuids;
    QPoint center;
    stream >> center;

    return PasteBufferAction::create(surfaceUuid, isBufferFormatted, std::move(buffer), std::move(usedUuids), center,
                                     root);
}

void HistorySerializer::serializeUnexposeControlAction(AxiomModel::UnexposeControlAction *action, QDataStream &stream) {
    stream << action->controlUuid();
    serializeDeleteObjectAction(action->deleteExposerAction(), stream);
}

std::unique_ptr<UnexposeControlAction>
    HistorySerializer::deserializeUnexposeControlAction(QDataStream &stream, uint32_t version,
                                                        AxiomModel::ModelRoot *root) {
    QUuid controlUuid;
    stream >> controlUuid;

    auto deleteObjectAction = deserializeDeleteObjectAction(stream, version, root);
    return UnexposeControlAction::create(controlUuid, std::move(deleteObjectAction), root);
}

void HistorySerializer::serializeAddGraphPointAction(AxiomModel::AddGraphPointAction *action, QDataStream &stream) {
    stream << action->controlUuid();
    stream << action->index();
    stream << action->time();
    stream << action->val();
}

std::unique_ptr<AddGraphPointAction> HistorySerializer::deserializeAddGraphPointAction(QDataStream &stream,
                                                                                       uint32_t version,
                                                                                       AxiomModel::ModelRoot *root) {
    QUuid controlUuid;
    stream >> controlUuid;
    uint8_t index;
    stream >> index;
    float time;
    stream >> time;
    float val;
    stream >> val;

    return AddGraphPointAction::create(controlUuid, index, time, val, root);
}

void HistorySerializer::serializeDeleteGraphPointAction(AxiomModel::DeleteGraphPointAction *action,
                                                        QDataStream &stream) {
    stream << action->controlUuid();
    stream << action->index();
    stream << action->time();
    stream << action->val();
    stream << action->tension();
    stream << action->state();
}

std::unique_ptr<DeleteGraphPointAction>
    HistorySerializer::deserializeDeleteGraphPointAction(QDataStream &stream, uint32_t version,
                                                         AxiomModel::ModelRoot *root) {
    QUuid controlUuid;
    stream >> controlUuid;
    uint8_t index;
    stream >> index;
    float time;
    stream >> time;
    float val;
    stream >> val;
    float tension;
    stream >> tension;
    uint8_t state;
    stream >> state;

    return DeleteGraphPointAction::create(controlUuid, index, time, val, tension, state, root);
}

void HistorySerializer::serializeMoveGraphPointAction(AxiomModel::MoveGraphPointAction *action, QDataStream &stream) {
    stream << action->controlUuid();
    stream << action->index();
    stream << action->oldTime();
    stream << action->oldValue();
    stream << action->newTime();
    stream << action->newValue();
}

std::unique_ptr<MoveGraphPointAction> HistorySerializer::deserializeMoveGraphPointAction(QDataStream &stream,
                                                                                         uint32_t version,
                                                                                         AxiomModel::ModelRoot *root) {
    QUuid controlUuid;
    stream >> controlUuid;
    uint8_t index;
    stream >> index;
    float oldTime;
    stream >> oldTime;
    float oldValue;
    stream >> oldValue;
    float newTime;
    stream >> newTime;
    float newValue;
    stream >> newValue;

    return MoveGraphPointAction::create(controlUuid, index, oldTime, oldValue, newTime, newValue, root);
}

void HistorySerializer::serializeSetGraphTagAction(AxiomModel::SetGraphTagAction *action, QDataStream &stream) {
    stream << action->controlUuid();
    stream << action->index();
    stream << action->oldTag();
    stream << action->newTag();
}

std::unique_ptr<SetGraphTagAction> HistorySerializer::deserializeSetGraphTagAction(QDataStream &stream,
                                                                                   uint32_t version,
                                                                                   AxiomModel::ModelRoot *root) {
    QUuid controlUuid;
    stream >> controlUuid;
    uint8_t index;
    stream >> index;
    uint8_t oldTag;
    stream >> oldTag;
    uint8_t newTag;
    stream >> newTag;

    return SetGraphTagAction::create(controlUuid, index, oldTag, newTag, root);
}

void HistorySerializer::serializeSetGraphTensionAction(AxiomModel::SetGraphTensionAction *action, QDataStream &stream) {
    stream << action->controlUuid();
    stream << action->index();
    stream << action->oldTension();
    stream << action->newTension();
}

std::unique_ptr<SetGraphTensionAction>
    HistorySerializer::deserializeSetGraphTensionAction(QDataStream &stream, uint32_t version,
                                                        AxiomModel::ModelRoot *root) {
    QUuid controlUuid;
    stream >> controlUuid;
    uint8_t index;
    stream >> index;
    float oldTension;
    stream >> oldTension;
    float newTension;
    stream >> newTension;

    return SetGraphTensionAction::create(controlUuid, index, oldTension, newTension, root);
}

void HistorySerializer::serializeSetNumRangeAction(AxiomModel::SetNumRangeAction *action, QDataStream &stream) {
    stream << action->uuid();
    stream << action->beforeMin();
    stream << action->beforeMax();
    stream << action->afterMin();
    stream << action->afterMax();
}

std::unique_ptr<SetNumRangeAction> HistorySerializer::deserializeSetNumRangeAction(QDataStream &stream,
                                                                                   uint32_t version,
                                                                                   AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    float beforeMin;
    stream >> beforeMin;
    float beforeMax;
    stream >> beforeMax;
    float afterMin;
    stream >> afterMin;
    float afterMax;
    stream >> afterMax;

    return SetNumRangeAction::create(uuid, beforeMin, beforeMax, afterMin, afterMax, root);
}
