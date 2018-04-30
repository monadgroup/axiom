#pragma once

#include <QtCore/QDataStream>
#include <memory>

#include "PoolObject.h"

namespace AxiomModel {

    class ModelRoot;

    class ModelObject : public PoolObject {
    public:
        enum class ModelType {
            SURFACE,
            NODE,
            CONTROL,
            LIBRARY_ENTRY,
            HISTORY_ACTION
        };

        ModelObject(ModelType modelType, const QUuid &uuid, const QUuid &parentUuid, ModelRoot *root);

        static std::unique_ptr<ModelObject> deserialize(QDataStream &stream, ModelRoot *root);

        virtual void serialize(QDataStream &stream) const;

        ModelType modelType() const { return _modelType; }

        ModelRoot *root() const { return _root; }

    private:
        ModelType _modelType;
        ModelRoot *_root;
    };

}
