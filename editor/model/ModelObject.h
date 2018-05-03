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
            SURFACE,
            CUSTOM_NODE,
            GROUP_NODE,
            IO_NODE,
            CONTROL,
            LIBRARY_ENTRY,
            HISTORY_ACTION
        };

        Event<> removed;
        Event<> cleanup;

        ModelObject(ModelType modelType, const QUuid &uuid, const QUuid &parentUuid, ModelRoot *root);

        static std::unique_ptr<ModelObject> deserialize(QDataStream &stream, ModelRoot *root);

        virtual void serialize(QDataStream &stream) const;

        ModelType modelType() const { return _modelType; }

        ModelRoot *root() const { return _root; }

    protected:
        void remove();

    private:
        ModelType _modelType;
        ModelRoot *_root;
    };

}
