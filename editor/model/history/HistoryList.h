#pragma once

#include <QtCore/QObject>
#include <memory>
#include <vector>

namespace AxiomModel {

    class HistoryOperation;

    class HistoryList : public QObject {
        Q_OBJECT

    public:

        ~HistoryList() override;

        void startAction(const std::string &name);

        void endAction(const std::string &name);

        template<class TR, class... Args>
        TR doAction(std::string name, TR (*cb)(Args...), Args... args) {
            startAction(name);
            auto result = cb(args...);
            endAction(name);
            return std::move(result);
        };

        void appendOperation(std::unique_ptr<HistoryOperation> operation);

        bool canUndo() const;

        bool canRedo() const;

    public slots:

        void undo();

        void redo();

    signals:

        void canUndoChanged(bool canUndo);

        void canRedoChanged(bool canRedo);

    private:

        struct HistoryAction {
            std::string name;
            std::vector<std::unique_ptr<HistoryOperation>> operations;
        };

        size_t stackPos = 0;
        std::vector<HistoryAction> stack;

        bool hasCurrentAction = false;
        HistoryAction currentAction;

    };

}
