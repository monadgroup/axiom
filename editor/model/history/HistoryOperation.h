#pragma once

#include <QtCore/QDataStream>
#include <memory>

namespace AxiomModel {

    class Project;

    class HistoryOperation {
    public:
        enum class Type {
            ADD_NODE,
            DELETE_NODE,
            MOVE_NODE,
            SIZE_NODE,
            RENAME_NODE,
            SHOW_HIDE_CONTROL_NAME,
            MOVE_CONTROL,
            SIZE_CONTROL,
            CONNECT_CONTROLS,
            DISCONNECT_CONTROLS
            /*CHANGE_NUM_VALUE,
            EXPOSE_CONTROL,
            ADD_WIRE,
            REMOVE_WIRE*/
        };

        HistoryOperation(bool needsRefresh, Type type, bool exec = true);

        static std::unique_ptr<HistoryOperation> deserialize(Type type, QDataStream &stream, Project *project);

        Type type() const { return _type; }

        bool exec() const { return _exec; }

        bool needsRefresh() const { return _needsRefresh; }

        virtual void forward() = 0;

        virtual void backward() = 0;

        virtual void serialize(QDataStream &stream) const = 0;

    private:
        bool _needsRefresh;
        Type _type;
        bool _exec;

    };

}
