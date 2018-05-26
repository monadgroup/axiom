#pragma once

#include <vector>
#include <memory>

#include "common/Event.h"
#include "actions/Action.h"

namespace AxiomModel {

    class ModelRoot;

    class Action;

    class HistoryList : public AxiomCommon::Hookable {
    public:
        AxiomCommon::Event<bool> canUndoChanged;
        AxiomCommon::Event<bool> canRedoChanged;
        AxiomCommon::Event<Action::ActionType> undoTypeChanged;
        AxiomCommon::Event<Action::ActionType> redoTypeChanged;
        AxiomCommon::Event<> rebuildRequested;
        AxiomCommon::Event<> stackChanged;

        size_t maxActions = 256;

        HistoryList();

        explicit HistoryList(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream);

        const std::vector<std::unique_ptr<Action>> &stack() const { return _stack; }

        size_t stackPos() const { return _stackPos; }

        void append(std::unique_ptr<Action> action, bool forward = true);

        bool canUndo() const;

        void undo();

        bool canRedo() const;

        void redo();

    private:
        size_t _stackPos = 0;
        std::vector<std::unique_ptr<Action>> _stack;
    };

}
