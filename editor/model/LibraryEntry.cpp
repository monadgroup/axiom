#include "LibraryEntry.h"
#include <utility>
#include "node/CustomNode.h"

using namespace AxiomModel;

LibraryEntry::LibraryEntry(QString name, std::set<QString> tags, AxiomModel::Project *project)
    : _name(std::move(name)), _baseUuid(QUuid::createUuid()), _modificationUuid(QUuid::createUuid()),
      _modificationDateTime(QDateTime::currentDateTime()), _tags(std::move(tags)), _schematic(this, project) {
}

LibraryEntry::LibraryEntry(QString name, QUuid baseUuid, QUuid modificationUuid, QDateTime modificationDateTime, std::set<QString> tags, Project *project)
    : _name(std::move(name)), _baseUuid(baseUuid), _modificationUuid(modificationUuid), _modificationDateTime(
    std::move(modificationDateTime)), _tags(std::move(tags)), _schematic(this, project) {
}

std::unique_ptr<LibraryEntry> LibraryEntry::deserialize(QDataStream &stream, Project *project) {
    QString name; stream >> name;
    QUuid baseUuid; stream >> baseUuid;
    QUuid modificationUuid; stream >> modificationUuid;
    QDateTime modificationDateTime; stream >> modificationDateTime;

    quint32 tagCount; stream >> tagCount;
    std::set<QString> tags;
    for (quint32 i = 0; i < tagCount; i++) {
        QString tagName; stream >> tagName;
        tags.emplace(tagName);
    }

    return std::make_unique<LibraryEntry>(name, baseUuid, modificationUuid, modificationDateTime, std::move(tags), project);
}

void LibraryEntry::serialize(QDataStream &stream) const {
    stream << _name;
    stream << _baseUuid;
    stream << _modificationUuid;
    stream << _modificationDateTime;
    stream << (quint32) _tags.size();
    for (const auto &tag : _tags) {
        stream << tag;
    }
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
    _modificationUuid = QUuid::createUuid();
    _modificationDateTime = QDateTime::currentDateTimeUtc();
}

void LibraryEntry::remove() {
    emit _schematic.removed();
    emit removed();
    emit cleanup();
}
