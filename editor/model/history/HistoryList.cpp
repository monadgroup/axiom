#include "HistoryList.h"

#include <cassert>

#include "HistoryOperation.h"
#include "../Project.h"
#include "../../util.h"

using namespace AxiomModel;

QString HistoryList::typeToString(AxiomModel::HistoryList::ActionType type) {
    switch (type) {
        case ActionType::NONE:
            return "";
        case ActionType::DELETE_SELECTED_ITEMS:
            return "Delete Selected Items";
        case ActionType::CREATE_GROUP_NODE:
            return "Create Group Node";
        case ActionType::CREATE_CUSTOM_NODE:
            return "Create Custom Node";
        case ActionType::MOVE_NODE:
            return "Move Node";
        case ActionType::SIZE_NODE:
            return "Resize Node";
        case ActionType::RENAME_NODE:
            return "Rename Node";
        case ActionType::SHOW_CONTROL_NAME:
            return "Show Control Name";
        case ActionType::HIDE_CONTROL_NAME:
            return "Hide Control Name";
        case ActionType::MOVE_CONTROL:
            return "Move Control";
        case ActionType::SIZE_CONTROL:
            return "Resize Control";
        case ActionType::CONNECT_CONTROL:
            return "Connect Controls";
        case ActionType::DISCONNECT_CONTROL:
            return "Disconnect Controls";
        case ActionType::DISCONNECT_ALL:
            return "Disconnect All Connections";
        case ActionType::CHANGE_VALUE:
            return "Change Value";
        case ActionType::CHANGE_MODE:
            return "Change Mode";
        case ActionType::PLACE_MODULE:
            return "Place Module";
    }

    unreachable;
}

HistoryList::HistoryList(AxiomModel::Project *project) : project(project) {

}

HistoryList::~HistoryList() = default;

void HistoryList::startAction(ActionType type) {
    assert(!hasCurrentAction);
    hasCurrentAction = true;
    currentAction.type = type;
    currentAction.operations.clear();
}

void HistoryList::endAction(ActionType type) {
    assert(hasCurrentAction);
    assert(currentAction.type == type);

    hasCurrentAction = false;

    // don't create the action if it's empty
    if (currentAction.operations.empty()) return;

    auto couldRedo = canRedo();

    // if any of the operations needed a refresh, do that now
    for (const auto &operation : currentAction.operations) {
        if (!operation->needsRefresh()) continue;
        project->build();
        break;
    }

    // remove items ahead of where we are
    _stack.erase(_stack.begin() + _stackPos, _stack.end());

    // if the stack is going to be longer than max size, remove the first item
    if (_stack.size() == maxActions) {
        _stack.erase(_stack.begin());
    } else _stackPos++;

    _stack.push_back(std::move(currentAction));

    // update undo/redo state
    if (_stackPos == 1) emit canUndoChanged(true);
    if (couldRedo) emit canRedoChanged(false);

    emit undoTypeChanged(type);
    emit redoTypeChanged(ActionType::NONE);
    emit stackChanged();
}

void HistoryList::cancelAction(AxiomModel::HistoryList::ActionType type) {
    assert(hasCurrentAction);
    assert(currentAction.type == type);

    hasCurrentAction = false;

    // undo all operations in the stack
    for (ssize_t i = currentAction.operations.size() - 1; i >= 0; i--) {
        currentAction.operations[i]->backward();
    }
}

void HistoryList::appendOperation(std::unique_ptr<AxiomModel::HistoryOperation> operation) {
    auto operationPtr = operation.get();
    if (hasCurrentAction) {
        currentAction.operations.push_back(std::move(operation));
    }
    if (operationPtr->exec()) {
        operationPtr->forward();
    }
}

bool HistoryList::canUndo() const {
    return _stackPos > 0;
}

bool HistoryList::canRedo() const {
    return _stackPos < _stack.size();
}

void HistoryList::serialize(QDataStream &stream) const {
    assert(!hasCurrentAction);

    stream << (quint32) _stack.size();
    stream << (quint32) _stackPos;
    for (const auto &action : _stack) {
        stream << (quint32) action.type;
        stream << (quint32) action.operations.size();
        for (const auto &operation : action.operations) {
            stream << (quint32) operation->type();
            operation->serialize(stream);
        }
    }
}

void HistoryList::deserialize(QDataStream &stream) {
    assert(!hasCurrentAction);

    _stack.clear();
    quint32 stackSize;
    stream >> stackSize;
    quint32 intPos;
    stream >> intPos;
    _stackPos = intPos;

    for (quint32 i = 0; i < stackSize; i++) {
        quint32 intActionType;
        stream >> intActionType;
        quint32 opCount;
        stream >> opCount;
        _stack.push_back(HistoryAction{(ActionType) intActionType, {}});

        auto &action = _stack.back();
        for (quint32 j = 0; j < opCount; j++) {
            quint32 intOpType;
            stream >> intOpType;
            action.operations.push_back(
                HistoryOperation::deserialize((HistoryOperation::Type) intOpType, stream, project));
        }
    }

    emit canUndoChanged(_stackPos > 0);
    emit canRedoChanged(_stackPos < _stack.size());
    emit undoTypeChanged(_stackPos > 0 ? _stack[_stackPos - 1].type : ActionType::NONE);
    emit redoTypeChanged(_stackPos < _stack.size() ? _stack[_stackPos].type : ActionType::NONE);
    emit stackChanged();
}

void HistoryList::undo() {
    if (hasCurrentAction || !canUndo()) return;

    _stackPos--;
    auto needsRefresh = false;
    auto &ops = _stack[_stackPos].operations;
    for (ssize_t i = ops.size() - 1; i >= 0; i--) {
        ops[i]->backward();
        if (ops[i]->needsRefresh()) needsRefresh = true;
    }

    if (needsRefresh) project->build();

    if (_stackPos == 0) emit canUndoChanged(false);
    if (_stackPos == _stack.size() - 1) emit canRedoChanged(true);

    emit undoTypeChanged(_stackPos == 0 ? ActionType::NONE : _stack[_stackPos - 1].type);
    emit redoTypeChanged(_stack[_stackPos].type);
    emit stackChanged();
}

void HistoryList::redo() {
    if (hasCurrentAction || !canRedo()) return;

    auto needsRefresh = false;
    auto &ops = _stack[_stackPos].operations;
    for (auto &op : ops) {
        op->forward();
        if (op->needsRefresh()) needsRefresh = true;
    }

    if (needsRefresh) project->build();

    _stackPos++;
    if (_stackPos == 1) emit canUndoChanged(true);
    if (_stackPos == _stack.size()) emit canRedoChanged(false);

    emit undoTypeChanged(_stack[_stackPos - 1].type);
    emit redoTypeChanged(_stackPos == _stack.size() ? ActionType::NONE : _stack[_stackPos].type);
    emit stackChanged();
}
