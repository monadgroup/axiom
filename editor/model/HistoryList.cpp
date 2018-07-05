#include "HistoryList.h"

#include "editor/compiler/interface/Transaction.h"

using namespace AxiomModel;

HistoryList::HistoryList(TransactionApplyer applyer) : applyTransaction(std::move(applyer)) {}

HistoryList::HistoryList(QDataStream &stream, ModelRoot *root) {
    uint32_t stackPos;
    stream >> stackPos;
    uint32_t stackSize;
    stream >> stackSize;

    _stackPos = stackPos;
    _stack.reserve(stackSize);
    for (uint32_t i = 0; i < stackSize; i++) {
        QByteArray actionBuffer;
        stream >> actionBuffer;
        QDataStream actionStream(&actionBuffer, QIODevice::ReadOnly);
        _stack.push_back(Action::deserialize(actionStream, root));
    }
}

void HistoryList::serialize(QDataStream &stream) {
    stream << (uint32_t) _stackPos;
    stream << (uint32_t) _stack.size();
    for (const auto &action : _stack) {
        QByteArray actionBuffer;
        QDataStream actionStream(&actionBuffer, QIODevice::WriteOnly);
        action->serialize(actionStream);
        stream << actionBuffer;
    }
}

void HistoryList::append(std::unique_ptr<AxiomModel::Action> action, bool forward) {
    // run the action forward
    if (forward) {
        MaximCompiler::Transaction transaction;
        action->forward(true, &transaction);
        applyTransaction(std::move(transaction));
    }

    // remove items ahead of where we are
    _stack.erase(_stack.begin() + _stackPos, _stack.end());

    // if the stack is going to be longer than max size, remove the first item
    if (_stack.size() == maxActions) {
        _stack.erase(_stack.begin());
    } else
        _stackPos++;

    _stack.push_back(std::move(action));

    stackChanged.trigger();
}

bool HistoryList::canUndo() const {
    return _stackPos > 0;
}

Action::ActionType HistoryList::undoType() const {
    if (canUndo()) return _stack[_stackPos - 1]->actionType();
    return Action::ActionType::NONE;
}

void HistoryList::undo() {
    if (!canUndo()) return;

    _stackPos--;
    auto undoAction = _stack[_stackPos].get();
    MaximCompiler::Transaction transaction;
    undoAction->backward(&transaction);
    applyTransaction(std::move(transaction));

    stackChanged.trigger();
}

bool HistoryList::canRedo() const {
    return _stackPos < _stack.size();
}

Action::ActionType HistoryList::redoType() const {
    if (canRedo()) return _stack[_stackPos]->actionType();
    return Action::ActionType::NONE;
}

void HistoryList::redo() {
    if (!canRedo()) return;

    auto redoAction = _stack[_stackPos].get();
    MaximCompiler::Transaction transaction;
    redoAction->forward(false, &transaction);
    _stackPos++;

    applyTransaction(std::move(transaction));

    stackChanged.trigger();
}
