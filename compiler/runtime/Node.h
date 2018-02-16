#pragma once

#include <vector>
#include <memory>

namespace MaximCodegen {
    class InstantiableFunction;
}

namespace MaximRuntime {

    class Control;

    class Surface;

    class Node {
    public:
        explicit Node(Surface *surface);

        Surface *surface() const { return _surface; }

        virtual std::vector<std::unique_ptr<Control>> &controls() = 0;

        virtual MaximCodegen::InstantiableFunction *getFunction() = 0;

    private:
        Surface *_surface;
    };

}
