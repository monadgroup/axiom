#pragma once

#include "Surface.h"
#include "Node.h"

namespace MaximRuntime {

    class GroupNode : public Node {
    public:
        GroupNode(MaximCodegen::MaximContext *context, Surface *surface);

        Surface &childSurface() const { return _surface; }

    private:
        Surface _surface;
    };

}
