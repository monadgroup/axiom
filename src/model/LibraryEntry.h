#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <set>

namespace AxiomModel {

    class Library;

    class LibraryEntry {
    public:
        Library *library;

        QString author;
        std::set<QString> tags;
    };

}
