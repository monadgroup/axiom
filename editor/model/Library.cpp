#include "Library.h"

#include <QtCore/QDataStream>

#include "LibraryEntry.h"
#include "../util.h"

using namespace AxiomModel;

Library::Library() = default;

Library::~Library() = default;

Library::Library(QDataStream &stream) {
    stream >> _activeTag;

    quint32 entryCount;
    stream >> entryCount;
    for (quint32 i = 0; i < entryCount; i++) {
        addEntry(LibraryEntry::deserialize(stream));
    }
}

void Library::serialize(QDataStream &stream) {
    stream << _activeTag;
    stream << (quint32) _entries.size();
    for (const auto &entry : _entries) {
        entry->serialize(stream);
    }
}

void Library::setActiveTag(const QString &activeTag) {
    if (activeTag != _activeTag) {
        _activeTag = activeTag;
        activeTagChanged.trigger(activeTag);
    }
}

std::vector<LibraryEntry *> Library::entries() const {
    std::vector<LibraryEntry*> result;
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

LibraryEntry* Library::findById(const QUuid &id) {
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
