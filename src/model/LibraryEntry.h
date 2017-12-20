#pragma once
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QSet>

namespace AxiomModel {

    class Library;

    class LibraryEntry {
    public:
        Library *library;

        QString author;
        QSet<QString> tags;
    };

}
