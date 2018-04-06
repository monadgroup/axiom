#pragma once

#include <QtCore/QObject>
#include <memory>
#include <vector>

namespace AxiomModel {

    class HistoryOperation;

    class Project;

    class HistoryList : public QObject {
        Q_OBJECT

    public:

        HistoryList(Project *project);

        ~HistoryList() override;

        void startAction(const std::string &name);

        void endAction(const std::string &name);

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

        Project *project;

        size_t stackPos = 0;
        std::vector<HistoryAction> stack;

        bool hasCurrentAction = false;
        HistoryAction currentAction;

    };

}
