#pragma once

#include <QtCore/QDataStream>
#include <memory>

#include "PoolObject.h"
#include "Event.h"

namespace AxiomModel {

    class ModelRoot;

    class ModelObject : public PoolObject {
    public:
        enum class ModelType {
            NODE_SURFACE,
            NODE,
            CONTROL_SURFACE,
            CONTROL,
            CONNECTION
        };

        Event<> removed;

        ModelObject(ModelType modelType, const QUuid &uuid, const QUuid &parentUuid, ModelRoot *root);

        static std::unique_ptr<ModelObject> deserialize(QDataStream &stream, const QUuid &parent, ModelRoot *root);

        static std::unique_ptr<ModelObject> deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, ModelRoot *root);

        virtual void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const;

        ModelType modelType() const { return _modelType; }

        ModelRoot *root() const { return _root; }

        void remove() override;

    private:
        ModelType _modelType;
        ModelRoot *_root;
    };

}
