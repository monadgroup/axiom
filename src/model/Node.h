#pragma once
#include <string>

namespace AxiomModel {

    class ISchematic;

    class Node {
    public:
        ISchematic *parent;

        std::string name;

        int x;
        int y;
        int width;
        int height;

        bool isSelected;
    };

}
