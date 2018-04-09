#include "Library.h"

#include "LibraryEntry.h"

#include <QtCore/QDataStream>

using namespace AxiomModel;

Library::~Library() = default;

void Library::serialize(QDataStream &stream) const {
    stream << (quint32) _entries.size();
    for (const auto &entry : _entries) {
        entry->serialize(stream);
    }
}

void Library::deserialize(QDataStream &stream) {
    quint32 entryCount; stream >> entryCount;
    for (quint32 i = 0; i < entryCount; i++) {
        auto newEntry = std::make_unique<LibraryEntry>("", std::set<QString>());
        newEntry->deserialize(stream);
        addEntry(std::move(newEntry));
    }
}

void Library::clear() {
    _entries.clear();
}

void Library::addEntry(std::unique_ptr<AxiomModel::LibraryEntry> entry) {
    auto entryPtr = entry.get();
    _entries.push_back(std::move(entry));
    emit entryAdded(entryPtr);
}
