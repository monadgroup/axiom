#pragma once

#include <QtCore/QDataStream>
#include <memory>

#include "common/Hookable.h"

namespace MaximCompiler {
    class Transaction;
}

namespace AxiomModel {

    class ModelRoot;

    class Action : public AxiomCommon::Hookable {
    public:
        enum class ActionType {
            NONE,
            COMPOSITE,
            DELETE_OBJECT,
            CREATE_CUSTOM_NODE,
            CREATE_GROUP_NODE,
            CREATE_PORTAL_NODE,
            CREATE_CONNECTION,
            MOVE_GRID_ITEM,
            SIZE_GRID_ITEM,
            RENAME_NODE,
            SET_CODE,
            CREATE_CONTROL,
            SET_NUM_MODE,
            SET_NUM_VALUE,
            SET_SHOW_NAME,
            EXPOSE_CONTROL,
            PASTE_BUFFER,
        };

        Action(ActionType actionType, ModelRoot *root);

        static QString typeToString(ActionType type);

        static std::unique_ptr<Action> deserialize(QDataStream &stream, ModelRoot *root);

        virtual void serialize(QDataStream &stream) const;

        ActionType actionType() const { return _actionType; }

        ModelRoot *root() const { return _root; }

        virtual void forward(bool first, std::vector<QUuid> &compileItems) = 0;

        virtual void backward(std::vector<QUuid> &compileItems) = 0;

    private:
        ActionType _actionType;
        ModelRoot *_root;
    };
}
