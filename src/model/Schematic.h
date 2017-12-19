#pragma once
#include <memory>
#include <vector>

namespace AxiomModel {

    class Node;

    class Schematic {
    public:
        std::vector<std::unique_ptr<Node>> nodes;

        virtual std::string getName() = 0;

        int panX;
        int panY;
    };

}
