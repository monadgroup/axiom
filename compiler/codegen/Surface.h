#pragma once

#include <vector>
#include <memory>

namespace MaximCodegen {

    class Node;

    class Surface {
    public:
        std::vector<std::unique_ptr<Node>> nodes;
    };

}
