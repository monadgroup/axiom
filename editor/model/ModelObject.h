#pragma once

#include <QtCore/QDataStream>
#include <memory>

#include "PoolObject.h"
#include "Sequence.h"
#include "common/Event.h"

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

        virtual void saveState() {}

        virtual void restoreState() {}

        virtual Sequence<ModelObject *> links();

        virtual Sequence<QUuid> deleteCompileLinks();

        virtual Sequence<QUuid> compileLinks();

        virtual void build(MaximCompiler::Transaction *transaction) {}

        void remove() override;

    private:
        ModelType _modelType;
        ModelRoot *_root;
    };
}
