#pragma once

#include "interface/Runtime.h"
#include "interface/Transaction.h"

namespace AxiomModel {
    class CustomNode;
}

namespace MaximCompiler {

    class CustomNodeMirBuilder {
    public:

        static void build(Runtime &runtime, Transaction &transaction, AxiomModel::CustomNode *node);
    };

}
