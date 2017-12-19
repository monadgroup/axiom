#pragma once
#include <string>
#include <set>

namespace AxiomModel {

    class Library;

    class LibraryEntry {
    public:
        Library *library;

        std::string author;
        std::set<std::string> tags;
    };

}
