#pragma once

#include <vector>
#include <memory>

#include "Event.h"
#include "actions/Action.h"

namespace AxiomModel {

    class ModelRoot;

    class Action;

    class HistoryList : public Hookable {
    public:
        Event<bool> canUndoChanged;
        Event<bool> canRedoChanged;
        Event<Action::ActionType> undoTypeChanged;
        Event<Action::ActionType> redoTypeChanged;

        size_t maxActions = 256;

        HistoryList();

        explicit HistoryList(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream);

        size_t stackPos() const { return _stackPos; }

        void append(std::unique_ptr<Action> action);

        bool canUndo() const;

        void undo();

        bool canRedo() const;

        void redo();

    private:
        size_t _stackPos = 0;
        std::vector<std::unique_ptr<Action>> _stack;
    };

}
