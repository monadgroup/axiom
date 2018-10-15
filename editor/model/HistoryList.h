#pragma once

#include <memory>
#include <vector>

#include "actions/Action.h"
#include "common/Event.h"
#include "common/TrackedObject.h"

namespace MaximCompiler {
    class Transaction;
}

namespace AxiomModel {

    class ModelRoot;

    class Action;

    class HistoryList {
    public:
        AxiomCommon::Event<> stackChanged;

        size_t maxActions = 256;

        using CompileApplyer = std::function<void(std::vector<QUuid>)>;

        explicit HistoryList(CompileApplyer applyer);

        HistoryList(size_t stackPos, std::vector<std::unique_ptr<Action>> stack, CompileApplyer applyer);

        const std::vector<std::unique_ptr<Action>> &stack() const { return _stack; }

        size_t stackPos() const { return _stackPos; }

        void append(std::unique_ptr<Action> action, bool forward = true);

        bool canUndo() const;

        Action::ActionType undoType() const;

        void undo();

        bool canRedo() const;

        Action::ActionType redoType() const;

        void redo();

        CompileApplyer &applyer() { return applyCompile; }

    private:
        size_t _stackPos = 0;
        std::vector<std::unique_ptr<Action>> _stack;
        CompileApplyer applyCompile;
    };
}
