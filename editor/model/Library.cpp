#include "Library.h"

#include "LibraryEntry.h"

using namespace AxiomModel;

Library::Library(AxiomModel::Project *project) : project(project) {

}

Library::~Library() = default;

void Library::serialize(QDataStream &stream) const {
    stream << _activeTag;
    stream << (quint32) _entries.size();
    for (const auto &entry : _entries) {
        entry->serialize(stream);
        entry->schematic().serialize(stream);
    }
}

void Library::deserialize(QDataStream &stream) {
    stream >> _activeTag;

    quint32 entryCount;
    stream >> entryCount;
    for (quint32 i = 0; i < entryCount; i++) {
        auto entry = LibraryEntry::deserialize(stream, project);
        auto entryPtr = entry.get();
        _entries.push_back(std::move(entry));
        entryPtr->schematic().deserialize(stream);
        emit entryAdded(entryPtr);
        connectEntry(entryPtr);
    }
}

void Library::clear() {
    _activeTag = "";
    while (!_entries.empty()) {
        _entries.front()->remove();
    }
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
    emit entryAdded(entryPtr);
    connectEntry(entryPtr);
}

void Library::addEntry(std::unique_ptr<AxiomModel::LibraryEntry> entry,
                       const std::vector<AxiomModel::GridItem *> &items, QPoint center) {
    auto entryPtr = entry.get();
    _entries.push_back(std::move(entry));
    entryPtr->schematic().copyIntoSelf(items, center);
    emit entryAdded(entryPtr);
    connectEntry(entryPtr);
}

void Library::connectEntry(AxiomModel::LibraryEntry *entryPtr) {
    for (const auto &tag : entryPtr->tags()) {
        addTag(tag);
    }
    connect(entryPtr, &AxiomModel::LibraryEntry::tagAdded,
            this, &Library::tagAdded);
    connect(entryPtr, &AxiomModel::LibraryEntry::tagRemoved,
            this, &Library::tagRemoved);
    connect(entryPtr, &AxiomModel::LibraryEntry::cleanup,
            this, [this, entryPtr]() { removeEntry(entryPtr); });
}

LibraryEntry* Library::findById(const QUuid &id) {
    for (const auto &entry : _entries) {
        if (entry->baseUuid() == id) return entry.get();
    }
    return nullptr;
}

void Library::setActiveTag(const QString &activeTag) {
    if (activeTag != _activeTag) {
        _activeTag = activeTag;
        emit activeTagChanged(activeTag);
    }
}

void Library::addTag(const QString &tag) {
    auto index = _tags.find(tag);
    if (index != _tags.end()) {
        index->second++;
    } else {
        _tags.emplace(tag, 1);
        emit tagAdded(tag);
    }
}

void Library::removeTag(const QString &tag) {
    auto index = _tags.find(tag);
    assert(index != _tags.end());

    index->second--;
    if (!index->second) {
        _tags.erase(index);
        if (_activeTag == tag) setActiveTag("");
        emit tagRemoved(tag);
    }
}

void Library::removeEntry(AxiomModel::LibraryEntry *entry) {
    for (auto i = _entries.begin(); i != _entries.end(); i++) {
        if (i->get() == entry) {
            _entries.erase(i);
            return;
        }
    }
}
