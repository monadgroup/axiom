#include "HistoryList.h"

#include <cassert>

#include "HistoryOperation.h"

using namespace AxiomModel;

HistoryList::~HistoryList() = default;

void HistoryList::startAction(const std::string &name) {
    assert(!hasCurrentAction);
    hasCurrentAction = true;
    currentAction.name = name;
    currentAction.operations.clear();
}

void HistoryList::endAction(const std::string &name) {
    assert(hasCurrentAction);
    assert(currentAction.name == name);

    hasCurrentAction = false;

    auto couldRedo = canRedo();

    // remove items ahead of where we are
    stack.erase(stack.begin() + stackPos, stack.end());
    stack.push_back(std::move(currentAction));

    // if the position went from 0 to 1, emit that we can undo
    stackPos++;
    if (stackPos == 1) emit canUndoChanged(true);
    if (couldRedo) emit canRedoChanged(false);
}

void HistoryList::appendOperation(std::unique_ptr<AxiomModel::HistoryOperation> operation) {
    assert(hasCurrentAction);
    auto operationPtr = operation.get();
    currentAction.operations.push_back(std::move(operation));
    operationPtr->forward();
}

bool HistoryList::canUndo() const {
    return stackPos > 0;
}

bool HistoryList::canRedo() const {
    return stackPos < stack.size();
}

void HistoryList::undo() {
    if (hasCurrentAction || !canUndo()) return;

    stackPos--;
    auto &ops = stack[stackPos].operations;
    for (size_t i = ops.size() - 1; i >= 0; i--) {
        ops[i]->backward();
    }

    if (stackPos == 0) emit canUndoChanged(false);
    if (stackPos == stack.size() - 1) emit canRedoChanged(true);
}

void HistoryList::redo() {
    if (hasCurrentAction || !canRedo()) return;

    auto &ops = stack[stackPos].operations;
    for (auto &op : ops) {
        op->forward();
    }

    stackPos++;
    if (stackPos == 1) emit canUndoChanged(true);
    if (stackPos == stack.size()) emit canRedoChanged(false);
}
