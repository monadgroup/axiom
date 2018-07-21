#pragma once

#include <QtCore/QDataStream>
#include <memory>

namespace AxiomModel {

    class Library;
    class LibraryEntry;
    class Project;

    namespace LibrarySerializer {
        void serialize(Library *library, QDataStream &stream);

        std::unique_ptr<Library> deserialize(QDataStream &stream, uint32_t version, Project *project);

        void serializeEntry(LibraryEntry *entry, QDataStream &stream);

        std::unique_ptr<LibraryEntry> deserializeEntry(QDataStream &stream, uint32_t version, Project *project);
    }
}
