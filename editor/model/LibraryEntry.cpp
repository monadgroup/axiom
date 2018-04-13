#include "LibraryEntry.h"

#include "node/CustomNode.h"

using namespace AxiomModel;

LibraryEntry::LibraryEntry(QString name, std::set<QString> tags) : _name(std::move(name)), _tags(std::move(tags)) {
    _schematic.addItem(std::make_unique<CustomNode>(&_schematic, "Test", QPoint(0, 0), QSize(2, 2)));
    addTag("Test" + (qrand() * 2 / RAND_MAX));
}

void LibraryEntry::serialize(QDataStream &stream) const {
    stream << _name;
    stream << baseUuid;
    stream << modificationUuid;
    stream << modificationDateTime;
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

    stream >> baseUuid;
    stream >> modificationUuid;
    stream >> modificationDateTime;

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

void LibraryEntry::modified() {
    modificationUuid = QUuid::createUuid();
    modificationDateTime = QDateTime::currentDateTimeUtc();
}
