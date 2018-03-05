#pragma once

#include "Surface.h"
#include "OutputNode.h"

namespace MaximRuntime {

    class RootSurface : public Surface {
    public:
        explicit RootSurface(Runtime *runtime);

        OutputNode output;
    };

}
