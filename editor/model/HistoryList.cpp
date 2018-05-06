#include "HistoryList.h"

using namespace AxiomModel;

HistoryList::HistoryList() = default;

HistoryList::HistoryList(QDataStream &stream, ModelRoot *root) {
    uint32_t stackPos; stream >> stackPos;
    uint32_t stackSize; stream >> stackSize;

    _stackPos = stackPos;
    _stack.reserve(stackSize);
    for (uint32_t i = 0; i < stackSize; i++) {
        _stack.push_back(Action::deserialize(stream, root));
    }
}

void HistoryList::serialize(QDataStream &stream) {
    stream << (uint32_t) _stackPos;
    stream << (uint32_t) _stack.size();
    for (const auto &action : _stack) {
        action->serialize(stream);
    }
}

void HistoryList::append(std::unique_ptr<AxiomModel::Action> action) {
    // run the action forward
    if (action->exec()) action->forward();

    auto couldRedo = canRedo();
    auto actionType = action->actionType();

    // remove items ahead of where we are
    _stack.erase(_stack.begin() + _stackPos, _stack.end());

    // if the stack is going to be longer than max size, remove the first item
    if (_stack.size() == maxActions) {
        _stack.erase(_stack.begin());
    } else _stackPos++;

    _stack.push_back(std::move(action));

    // update undo/redo state
    if (_stackPos == 1) canUndoChanged.trigger(true);
    if (couldRedo) canRedoChanged.trigger(false);

    undoTypeChanged.trigger(actionType);
    redoTypeChanged.trigger(Action::ActionType::NONE);
}

bool HistoryList::canUndo() const {
    return _stackPos > 0;
}

void HistoryList::undo() {
    if (!canUndo()) return;

    _stackPos--;
    _stack[_stackPos]->backward();

    if (_stackPos == 0) canUndoChanged.trigger(false);
    if (_stackPos == _stack.size() - 1) canRedoChanged.trigger(true);

    undoTypeChanged.trigger(_stackPos == 0 ? Action::ActionType::NONE : _stack[_stackPos - 1]->actionType());
    redoTypeChanged.trigger(_stack[_stackPos]->actionType());
}

bool HistoryList::canRedo() const {
    return _stackPos < _stack.size();
}

void HistoryList::redo() {
    if (!canRedo()) return;

    _stack[_stackPos]->forward();
    _stackPos++;

    if (_stackPos == 1) canUndoChanged.trigger(true);
    if (_stackPos == _stack.size()) canRedoChanged.trigger(false);

    undoTypeChanged.trigger(_stack[_stackPos - 1]->actionType());
    redoTypeChanged.trigger(_stackPos == _stack.size() ? Action::ActionType::NONE : _stack[_stackPos]->actionType());
}
