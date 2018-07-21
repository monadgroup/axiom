#include "Library.h"

#include <QtCore/QDataStream>
#include <QtCore/QMap>
#include <QtWidgets/QMessageBox>

#include "../util.h"
#include "LibraryEntry.h"

using namespace AxiomModel;

Library::Library() = default;

Library::~Library() = default;

Library::Library(QString activeTag, QString activeSearch,
                 std::vector<std::unique_ptr<AxiomModel::LibraryEntry>> entries)
    : _entries(std::move(entries)), _activeTag(std::move(activeTag)), _activeSearch(std::move(activeSearch)) {}

void Library::import(
    AxiomModel::Library *library,
    const std::function<Library::ConflictResolution(LibraryEntry *, LibraryEntry *)> &resolveConflict) {
    // create a mapping of uuid/entry pairs for checking conflicts
    QMap<QUuid, LibraryEntry *> currentEntries;
    for (const auto &entry : _entries) {
        currentEntries.insert(entry->baseUuid(), entry.get());
    }

    // merge, checking for duplicates
    // if we hit a duplicate, we have two possible actions:
    //   - if the modification UUID is the same as the local one, they're the same and we can just keep what we've got
    //   - if not, there's a desync, and we need to ask the user which one they want to keep
    for (auto &entry : library->_entries) {
        auto currentEntry = currentEntries.find(entry->baseUuid());

        if (currentEntry == currentEntries.end()) {
            // no conflict, just add the entry
            addEntry(std::move(entry));
        } else {
            auto curEntry = currentEntry.value();
            if (entry->modificationUuid() != curEntry->modificationUuid()) {
                // different modification IDs, one of them has to go
                auto resolution = resolveConflict(curEntry, entry.get());

                switch (resolution) {
                case ConflictResolution::KEEP_OLD:
                    // no action needed
                    break;
                case ConflictResolution::KEEP_NEW:
                    currentEntries.remove(curEntry->baseUuid());
                    curEntry->remove();
                    addEntry(std::move(entry));
                    break;
                case ConflictResolution::KEEP_BOTH:
                    // todo
                    break;
                }
            }
        }
    }
}

void Library::setActiveTag(const QString &activeTag) {
    if (activeTag != _activeTag) {
        _activeTag = activeTag;
        activeTagChanged.trigger(activeTag);
    }
}

void Library::setActiveSearch(const QString &search) {
    if (search != _activeSearch) {
        _activeSearch = search;
        activeSearchChanged.trigger(search);
    }
}

std::vector<LibraryEntry *> Library::entries() const {
    std::vector<LibraryEntry *> result;
    result.reserve(_entries.size());
    for (const auto &entry : _entries) {
        result.push_back(entry.get());
    }
    return std::move(result);
}

QStringList Library::tags() const {
    QStringList result;
    for (const auto &pair : _tags) {
        result << pair.first;
    }
    return std::move(result);
}

void Library::addEntry(std::unique_ptr<AxiomModel::LibraryEntry> entry) {
    auto entryPtr = entry.get();
    _entries.push_back(std::move(entry));
    entryAdded.trigger(entryPtr);

    for (const auto &tag : entryPtr->tags()) {
        addTag(tag);
    }
    entryPtr->tagAdded.connect(this, &Library::addTag);
    entryPtr->tagRemoved.connect(this, &Library::removeTag);
    entryPtr->cleanup.connect(this, [this, entryPtr]() { removeEntry(entryPtr); });
}

LibraryEntry *Library::findById(const QUuid &id) {
    for (const auto &entry : _entries) {
        if (entry->baseUuid() == id) return entry.get();
    }
    return nullptr;
}

void Library::addTag(const QString &tag) {
    auto index = _tags.find(tag);
    if (index != _tags.end()) {
        index->second++;
    } else {
        _tags.emplace(tag, 1);
        tagAdded.trigger(tag);
    }
}

void Library::removeTag(const QString &tag) {
    auto index = _tags.find(tag);
    assert(index != _tags.end());

    index->second--;
    if (!index->second) {
        _tags.erase(index);
        if (_activeTag == tag) setActiveTag("");
        tagRemoved.trigger(tag);
    }
}

void Library::removeEntry(AxiomModel::LibraryEntry *entry) {
    for (auto i = _entries.begin(); i != _entries.end(); i++) {
        if (i->get() == entry) {
            _entries.erase(i);
            return;
        }
    }
    unreachable;
}
