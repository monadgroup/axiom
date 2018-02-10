#pragma once

#include "Node.h"
#include "Surface.h"

namespace MaximCodegen {

    class GroupNode : public Node {
    public:
        Surface surface;
    };

}
