#pragma once
#include <map>
#include <vector>
#include <memory>

namespace AxiomModel {

    class LibraryEntry;

    class Library {
    public:
        std::vector<std::unique_ptr<LibraryEntry>> entries;
        std::map<std::string, std::vector<LibraryEntry*>> index;
    };

}
