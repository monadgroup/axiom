#pragma once

#include <memory>
#include <vector>

#include "actions/Action.h"
#include "common/Event.h"

namespace MaximCompiler {
    class Transaction;
}

namespace AxiomModel {

    class ModelRoot;

    class Action;

    class HistoryList : public AxiomCommon::Hookable {
    public:
        AxiomCommon::Event<> stackChanged;

        size_t maxActions = 256;

        using CompileApplyer = std::function<void(std::vector<QUuid>)>;

        HistoryList(CompileApplyer applyer);

        explicit HistoryList(QDataStream &stream, ModelRoot *root);

        void serialize(QDataStream &stream);

        const std::vector<std::unique_ptr<Action>> &stack() const { return _stack; }

        size_t stackPos() const { return _stackPos; }

        void append(std::unique_ptr<Action> action, bool forward = true);

        bool canUndo() const;

        Action::ActionType undoType() const;

        void undo();

        bool canRedo() const;

        Action::ActionType redoType() const;

        void redo();

    private:
        size_t _stackPos = 0;
        std::vector<std::unique_ptr<Action>> _stack;
        CompileApplyer applyCompile;
    };
}
