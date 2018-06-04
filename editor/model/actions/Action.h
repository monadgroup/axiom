#pragma once

#include <memory>
#include <QtCore/QDataStream>

#include "common/Hookable.h"

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
            CREATE_AUTOMATION_NODE
        };

        Action(ActionType actionType, ModelRoot *root);

        static QString typeToString(ActionType type);

        static std::unique_ptr<Action> deserialize(QDataStream &stream, ModelRoot *root);

        virtual void serialize(QDataStream &stream) const;

        ActionType actionType() const { return _actionType; }

        ModelRoot *root() const { return _root; }

        virtual bool forward(bool first) = 0;

        virtual bool backward() = 0;

    private:
        ActionType _actionType;
        ModelRoot *_root;
    };

}
