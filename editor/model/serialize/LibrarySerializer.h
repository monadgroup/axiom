#pragma once

#include <QtCore/QDataStream>
#include <memory>

namespace AxiomModel {

    class Library;
    class LibraryEntry;
    class Project;

    namespace LibrarySerializer {
        void serialize(Library *library, QDataStream &stream, bool includeBuiltin);

        std::unique_ptr<Library> deserialize(QDataStream &stream, uint32_t version, bool isBuiltin);

        void serializeEntry(LibraryEntry *entry, QDataStream &stream);

        std::unique_ptr<LibraryEntry> deserializeEntry(QDataStream &stream, uint32_t version, bool isBuiltin);

        template<class Iterator>
        void serializeEntries(uint32_t count, Iterator begin, Iterator end, QDataStream &stream) {
            stream << count;
            for (auto entryIter = begin; entryIter != end; entryIter++) {
                serializeEntry(*entryIter, stream);
            }
        }
    }
}
