#include "LibrarySerializer.h"

#include "../Library.h"
#include "../LibraryEntry.h"
#include "ModelObjectSerializer.h"

using namespace AxiomModel;

void LibrarySerializer::serialize(AxiomModel::Library *library, QDataStream &stream) {
    auto entries = library->entries();
    serializeEntries((uint32_t) entries.size(), entries.begin(), entries.end(), stream);
}

std::unique_ptr<Library> LibrarySerializer::deserialize(QDataStream &stream, uint32_t version) {
    // Schema version 5 (Axiom version 0.4.0) stored an active tag and active search, since libraries were tried to
    // projects.
    if (version < 5) {
        QString dummy;
        stream >> dummy;
        stream >> dummy;
    }

    std::vector<std::unique_ptr<LibraryEntry>> entries;
    uint32_t entryCount;
    stream >> entryCount;
    entries.reserve(entryCount);
    for (uint32_t i = 0; i < entryCount; i++) {
        entries.push_back(deserializeEntry(stream, version));
    }

    return std::make_unique<Library>("", "", std::move(entries));
}

void LibrarySerializer::serializeEntry(AxiomModel::LibraryEntry *entry, QDataStream &stream) {
    stream << entry->name();
    stream << entry->baseUuid();
    stream << entry->modificationUuid();
    stream << entry->modificationDateTime();
    stream << (uint32_t) entry->tags().size();
    for (const auto &tag : entry->tags()) {
        stream << tag;
    }
    ModelObjectSerializer::serializeRoot(entry->root(), false, stream);
}

std::unique_ptr<LibraryEntry> LibrarySerializer::deserializeEntry(QDataStream &stream, uint32_t version) {
    QString name;
    stream >> name;
    QUuid baseUuid;
    stream >> baseUuid;
    QUuid modificationUuid;
    stream >> modificationUuid;
    QDateTime modificationDateTime;
    stream >> modificationDateTime;

    uint32_t tagSize;
    stream >> tagSize;
    std::set<QString> tags;
    for (uint32_t i = 0; i < tagSize; i++) {
        QString tag;
        stream >> tag;
        tags.emplace(tag);
    }

    // 0.4.0 (schema version 5) removed history from being serialized in the library
    auto deserializeHistory = version < 5;

    auto root = ModelObjectSerializer::deserializeRoot(stream, deserializeHistory, true, version);
    return LibraryEntry::create(std::move(name), baseUuid, modificationUuid, modificationDateTime, std::move(tags),
                                std::move(root));
}
