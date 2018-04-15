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
            NONE,
            DELETE_SELECTED_ITEMS,
            CREATE_GROUP_NODE,
            CREATE_CUSTOM_NODE,
            MOVE_NODE,
            SIZE_NODE,
            RENAME_NODE,
            SHOW_CONTROL_NAME,
            HIDE_CONTROL_NAME,
            MOVE_CONTROL,
            SIZE_CONTROL,
            CONNECT_CONTROL,
            DISCONNECT_CONTROL,
            DISCONNECT_ALL,
            CHANGE_VALUE,
            CHANGE_MODE,
            PLACE_MODULE
        };

        struct HistoryAction {
            ActionType type;
            std::vector<std::unique_ptr<HistoryOperation>> operations;
        };

        size_t maxActions = 256;

        static QString typeToString(ActionType type);

        explicit HistoryList(Project *project);

        ~HistoryList() override;

        size_t stackPos() const { return _stackPos; }

        const std::vector<HistoryAction> &stack() const { return _stack; }

        void startAction(ActionType type);

        void endAction(ActionType type);

        void cancelAction(ActionType type);

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

        void undoTypeChanged(ActionType newType);

        void redoTypeChanged(ActionType newType);

        void stackChanged();

    private:

        Project *project;

        size_t _stackPos = 0;
        std::vector<HistoryAction> _stack;

        bool hasCurrentAction = false;
        HistoryAction currentAction;

    };

}
