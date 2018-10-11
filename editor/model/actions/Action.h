#pragma once

#include <QtCore/QDataStream>
#include <memory>

#include "common/TrackedObject.h"

namespace MaximCompiler {
    class Transaction;
}

namespace AxiomModel {

    class ModelRoot;

    class Action : public AxiomCommon::TrackedObject {
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
            UNEXPOSE_CONTROL,
            RENAME_CONTROL,
            ADD_GRAPH_POINT,
            DELETE_GRAPH_POINT,
            MOVE_GRAPH_POINT,
            SET_GRAPH_TAG,
            SET_GRAPH_TENSION,
            SET_NUM_RANGE
        };

        Action(ActionType actionType, ModelRoot *root);

        static QString typeToString(ActionType type);

        ActionType actionType() const { return _actionType; }

        ModelRoot *root() const { return _root; }

        virtual void forward(bool first, std::vector<QUuid> &compileItems) = 0;

        virtual void backward(std::vector<QUuid> &compileItems) = 0;

    private:
        ActionType _actionType;
        ModelRoot *_root;
    };
}
