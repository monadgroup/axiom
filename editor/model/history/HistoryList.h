#pragma once

#include <QtCore/QObject>
#include <memory>
#include <vector>

#define DO_ACTION(history, name, block) (history).startAction((name)); (block); (history).endAction((name))

namespace AxiomModel {

    class HistoryOperation;

    class Project;

    class HistoryList : public QObject {
        Q_OBJECT

    public:

        enum class ActionType {
            DELETE_SELECTED_ITEMS,
            CREATE_GROUP_NODE,
            CREATE_CUSTOM_NODE,
            MOVE_NODE
        };

        size_t maxActions = 256;

        explicit HistoryList(Project *project);

        ~HistoryList() override;

        void startAction(ActionType type);

        void endAction(ActionType type);

        void appendOperation(std::unique_ptr<HistoryOperation> operation);

        bool canUndo() const;

        bool canRedo() const;

        void serialize(QDataStream &stream) const;

        void deserialize(QDataStream &stream);

    public slots:

        void undo();

        void redo();

    signals:

        void canUndoChanged(bool canUndo);

        void canRedoChanged(bool canRedo);

    private:

        struct HistoryAction {
            ActionType type;
            std::vector<std::unique_ptr<HistoryOperation>> operations;
        };

        Project *project;

        size_t stackPos = 0;
        std::vector<HistoryAction> stack;

        bool hasCurrentAction = false;
        HistoryAction currentAction;

    };

}
