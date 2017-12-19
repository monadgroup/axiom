#pragma once
#include <string>

#include "Library.h"
#include "RootSchematic.h"

namespace AxiomModel {

    class Project {
    public:
        std::string path;

        Library library;
        RootSchematic root;
    };

}
