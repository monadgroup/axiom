#include "HistoryList.h"

#include <cassert>

#include "HistoryOperation.h"
#include "../Project.h"

using namespace AxiomModel;

HistoryList::HistoryList(AxiomModel::Project *project) : project(project) {

}

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
    auto needsRefresh = false;
    auto &ops = stack[stackPos].operations;
    for (ssize_t i = ops.size() - 1; i >= 0; i--) {
        ops[i]->backward();
        if (ops[i]->needsRefresh()) needsRefresh = true;
    }

    if (needsRefresh) project->build();

    if (stackPos == 0) emit canUndoChanged(false);
    if (stackPos == stack.size() - 1) emit canRedoChanged(true);
}

void HistoryList::redo() {
    if (hasCurrentAction || !canRedo()) return;

    auto needsRefresh = false;
    auto &ops = stack[stackPos].operations;
    for (auto &op : ops) {
        op->forward();
        if (op->needsRefresh()) needsRefresh = true;
    }

    if (needsRefresh) project->build();

    stackPos++;
    if (stackPos == 1) emit canUndoChanged(true);
    if (stackPos == stack.size()) emit canRedoChanged(false);
}
