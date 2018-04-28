#pragma once

#include <QtCore/QDataStream>
#include <memory>

namespace AxiomModel {

    class Project;

    class HistoryOperation {
    public:
        enum class Type {
            ADD_NODE,
            CREATE_NODE,
            DELETE_NODE,
            MOVE_NODE,
            SIZE_NODE,
            RENAME_NODE,
            SHOW_HIDE_CONTROL_NAME,
            MOVE_CONTROL,
            SIZE_CONTROL,
            CONNECT_CONTROLS,
            DISCONNECT_CONTROLS,
            CHANGE_NUM_VAL,
            CHANGE_NUM_MODE,
            CHANGE_CODE
        };

        enum class ReadLevel {
            SURFACE,
            NODE,
            CONTROL,
            CODE
        };

        static QString typeToString(Type type);

        HistoryOperation(bool needsRefresh, ReadLevel level, Type type, bool exec = true);

        static std::unique_ptr<HistoryOperation> deserialize(Type type, QDataStream &stream, Project *project);

        Type type() const { return _type; }

        ReadLevel level() const { return _level; }

        bool exec() const { return _exec; }

        bool needsRefresh() const { return _needsRefresh; }

        virtual void forward() = 0;

        virtual void backward() = 0;

        virtual void serialize(QDataStream &stream) const = 0;

    private:
        bool _needsRefresh;
        ReadLevel _level;
        Type _type;
        bool _exec;

    };

}
