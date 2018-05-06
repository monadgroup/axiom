#pragma once

#include <memory>
#include <QtCore/QDataStream>

namespace AxiomModel {

    class ModelRoot;

    class Action {
    public:
        enum class ActionType {
            DELETE_OBJECT,
            CREATE_CUSTOM_NODE,
            CREATE_GROUP_NODE
        };

        Action(ActionType actionType, ModelRoot *root);

        std::unique_ptr<Action> deserialize(QDataStream &stream, ModelRoot *root);

        virtual void serialize(QDataStream &stream) const;

        ActionType actionType() const { return _actionType; }

        ModelRoot *root() const { return _root; }

        virtual void forward() const = 0;

        virtual void backward() const = 0;

    private:
        ActionType _actionType;
        ModelRoot *_root;
    };

}
