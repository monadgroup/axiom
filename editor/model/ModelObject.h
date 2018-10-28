#pragma once

#include <QtCore/QDataStream>
#include <memory>

#include "PoolObject.h"
#include "common/Event.h"
#include "common/Sequence.h"

namespace MaximCompiler {
    class Transaction;
}

namespace AxiomModel {

    class ModelRoot;

    class ReferenceMapper;

    class ModelObject : public PoolObject {
    public:
        enum class ModelType { NODE_SURFACE, NODE, CONTROL_SURFACE, CONTROL, CONNECTION };

        AxiomCommon::Event<> removed;

        ModelObject(ModelType modelType, const QUuid &uuid, const QUuid &parentUuid, ModelRoot *root);

        ModelType modelType() const { return _modelType; }

        ModelRoot *root() const { return _root; }

        bool isDirty() const { return _isDirty; }

        void clearDirty() { _isDirty = false; }

        virtual void saveState() {}

        virtual void restoreState() {}

        virtual void doRuntimeUpdate() {}

        virtual AxiomCommon::BoxedSequence<ModelObject *> links();

        virtual void build(MaximCompiler::Transaction *transaction) {}

        void remove() override;

    protected:
        void setDirty() { _isDirty = true; }

    private:
        ModelType _modelType;
        ModelRoot *_root;
        bool _isDirty = true;
    };
}
