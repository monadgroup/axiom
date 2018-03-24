#pragma once

#include "Surface.h"
#include "IONode.h"

namespace MaximRuntime {

    class RootSurface : public Surface {
    public:
        explicit RootSurface(Runtime *runtime);

        void *getValuePtr(void *parentCtx) override;

        void addExitNodes(std::set<Node *> &queue) override;

        IONode input;
        IONode output;
    };

}
