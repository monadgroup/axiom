#include "LibraryEntry.h"

using namespace AxiomModel;

LibraryEntry::LibraryEntry(QString name, std::set<QString> tags) : _name(std::move(name)), _tags(std::move(tags)) {

}

void LibraryEntry::serialize(QDataStream &stream) const {
    stream << _name;
    stream << (quint32) _tags.size();
    for (const auto &tag : _tags) {
        stream << tag;
    }
    _schematic.serialize(stream);
}

void LibraryEntry::deserialize(QDataStream &stream) {
    QString name;
    stream >> name;
    setName(name);

    quint32 tagCount;
    stream >> tagCount;
    for (quint32 i = 0; i < tagCount; i++) {
        QString tagName;
        stream >> tagName;
        addTag(tagName);
    }

    _schematic.deleteAll();
    _schematic.deserialize(stream);
}

void LibraryEntry::setName(const QString &name) {
    if (name != _name) {
        _name = name;
        emit nameChanged(name);
    }
}

void LibraryEntry::addTag(const QString &tag) {
    if (_tags.insert(tag).second) emit tagAdded(tag);
}

void LibraryEntry::removeTag(const QString &tag) {
    if (_tags.erase(tag)) emit tagRemoved(tag);
}
