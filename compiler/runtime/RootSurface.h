#pragma once

#include "Surface.h"
#include "OutputNode.h"

namespace MaximRuntime {

    class RootSurface : public Surface {
    public:
        explicit RootSurface(Runtime *runtime);

        void *getValuePtr(void *parentCtx) override;

        OutputNode output;
    };

}
