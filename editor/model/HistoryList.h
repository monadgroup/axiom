#pragma once

#include <vector>
#include <memory>

#include "Event.h"
#include "actions/Action.h"

namespace AxiomModel {

    class ModelRoot;

    class Action;

    class HistoryList {
    public:
        Event<bool> canUndoChanged;
        Event<bool> canRedoChanged;
        Event<Action::ActionType> undoTypeChanged;
        Event<Action::ActionType> redoTypeChanged;

        size_t maxActions = 256;

        explicit HistoryList(ModelRoot *root);

        ModelRoot *root() const { return _root; }

        size_t stackPos() const { return _stackPos; }

        void append(std::unique_ptr<Action> action);

        bool canUndo() const;

        void undo();

        bool canRedo() const;

        void redo();

    private:
        ModelRoot *_root;
        size_t _stackPos = 0;
        std::vector<std::unique_ptr<Action>> _stack;
    };

}
