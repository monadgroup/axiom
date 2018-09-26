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
    : _activeTag(std::move(activeTag)), _activeSearch(std::move(activeSearch)) {
    // add all entries through the normal path, so we assign event handlers to them correctly
    _entries.reserve(entries.size());
    for (auto &entry : entries) {
        addEntry(std::move(entry));
    }
}

void Library::import(
    AxiomModel::Library *library,
    const std::function<Library::ConflictResolution(LibraryEntry *, LibraryEntry *)> &resolveConflict) {
    // create a mapping of uuid/entry pairs for checking conflicts
    QMap<QUuid, LibraryEntry *> currentEntries;
    for (const auto &entry : _entries) {
        currentEntries.insert(entry->baseUuid(), entry.get());
    }

    // build a list of merge actions, checking for duplicates
    // if we hit a duplicate, we have two possible actions:
    //   - if the modification UUID is the same as the local one, they're the same and we can just keep what we've got
    //   - if not, there's a desync, and we need to ask the user which one they want to keep
    std::vector<std::unique_ptr<LibraryEntry>> addEntries;
    std::vector<LibraryEntry *> removeEntries;
    std::vector<std::pair<LibraryEntry *, LibraryEntry *>> conflictRenameEntries;
    for (auto &entry : library->_entries) {
        auto currentEntry = currentEntries.find(entry->baseUuid());

        if (currentEntry == currentEntries.end()) {
            // no conflict, just add the entry
            addEntries.push_back(std::move(entry));
        } else {
            auto curEntry = currentEntry.value();
            if (entry->modificationUuid() != curEntry->modificationUuid()) {
                // different modification IDs, one of them has to go
                auto resolution = resolveConflict(curEntry, entry.get());

                switch (resolution) {
                case ConflictResolution::CANCEL:
                    // cancel everything by just exiting - since merging is transactional, this works fine
                    return;
                case ConflictResolution::KEEP_OLD:
                    // no action needed
                    break;
                case ConflictResolution::KEEP_NEW:
                    removeEntries.push_back(curEntry);
                    addEntries.push_back(std::move(entry));
                    break;
                case ConflictResolution::KEEP_BOTH:
                    conflictRenameEntries.emplace_back(curEntry, entry.get());
                    addEntries.push_back(std::move(entry));
                    break;
                }
            }
        }
    }

    // apply the transaction, making sure it's ordered so we don't have any duplicates or dangling references
    for (const auto &conflict : conflictRenameEntries) {
        auto originalEntry = conflict.first;
        auto newEntry = conflict.second;
        originalEntry->setName(originalEntry->name() + " (original)");
        newEntry->setName(newEntry->name() + " (imported)");

        // old entry becomes an entirely different entry, meaning if the library is imported in the future again,
        // the conflict won't show up
        originalEntry->setBaseUuid(QUuid());
    }

    for (const auto &removeEntry : removeEntries) {
        currentEntries.remove(removeEntry->baseUuid());
        removeEntry->remove();
    }

    for (auto &newEntry : addEntries) {
        addEntry(std::move(newEntry));
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
    return result;
}

QStringList Library::tags() const {
    QStringList result;
    for (const auto &pair : _tags) {
        result << pair.first;
    }
    return result;
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
    entryPtr->changed.connect(&changed);
    entryPtr->cleanup.connect(this, [this, entryPtr]() { removeEntry(entryPtr); });

    changed.trigger();
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
        tagRemoved.trigger(tag);
    }
}

void Library::removeEntry(AxiomModel::LibraryEntry *entry) {
    for (const auto &tag : entry->tags()) {
        removeTag(tag);
    }

    for (auto i = _entries.begin(); i != _entries.end(); i++) {
        if (i->get() == entry) {
            _entries.erase(i);
            return;
        }
    }
    unreachable;
}
