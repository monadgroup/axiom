#pragma once

#include <memory>
#include <QtCore/QDataStream>

#include "../Hookable.h"

namespace AxiomModel {

    class ModelRoot;

    class Action : public Hookable {
    public:
        enum class ActionType {
            NONE,
            DELETE_OBJECT,
            CREATE_CUSTOM_NODE,
            CREATE_GROUP_NODE,
            CREATE_CONNECTION
        };

        Action(ActionType actionType, bool exec, ModelRoot *root);

        static QString typeToString(ActionType type);

        static std::unique_ptr<Action> deserialize(QDataStream &stream, ModelRoot *root);

        virtual void serialize(QDataStream &stream) const;

        ActionType actionType() const { return _actionType; }

        ModelRoot *root() const { return _root; }

        bool exec() const { return _exec; }

        virtual void forward() const = 0;

        virtual void backward() const = 0;

    private:
        ActionType _actionType;
        bool _exec;
        ModelRoot *_root;
    };

}
