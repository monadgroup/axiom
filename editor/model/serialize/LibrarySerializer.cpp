#include "LibrarySerializer.h"

#include "../Library.h"
#include "../LibraryEntry.h"
#include "ModelObjectSerializer.h"

using namespace AxiomModel;

void LibrarySerializer::serialize(AxiomModel::Library *library, QDataStream &stream) {
    stream << library->activeTag();
    stream << library->activeSearch();

    auto entries = library->entries();
    stream << (uint32_t) entries.size();
    for (const auto &entry : entries) {
        serializeEntry(entry, stream);
    }
}

std::unique_ptr<Library> LibrarySerializer::deserialize(QDataStream &stream, uint32_t version, Project *project) {
    QString activeTag;
    stream >> activeTag;
    QString activeSearch;
    stream >> activeSearch;

    std::vector<std::unique_ptr<LibraryEntry>> entries;
    uint32_t entryCount;
    stream >> entryCount;
    entries.reserve(entryCount);
    for (uint32_t i = 0; i < entryCount; i++) {
        entries.push_back(deserializeEntry(stream, version, project));
    }

    return std::make_unique<Library>(std::move(activeTag), std::move(activeSearch), std::move(entries));
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

std::unique_ptr<LibraryEntry> LibrarySerializer::deserializeEntry(QDataStream &stream, uint32_t version,
                                                                  AxiomModel::Project *project) {
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

    auto root = ModelObjectSerializer::deserializeRoot(stream, false, version, project);
    return LibraryEntry::create(std::move(name), baseUuid, modificationUuid, modificationDateTime, std::move(tags),
                                std::move(root));
}
