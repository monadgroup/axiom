#include "LibraryEntry.h"

#include "ModelRoot.h"

using namespace AxiomModel;

LibraryEntry::LibraryEntry(QString name, const QUuid &baseUuid, const QUuid &modificationUuid,
                           const QDateTime &modificationDateTime, std::set<QString> tags, std::unique_ptr<AxiomModel::ModelRoot> root)
   : _name(std::move(name)), _baseUuid(baseUuid), _modificationUuid(modificationUuid),
   _modificationDateTime(modificationDateTime), _tags(std::move(tags)), _root(std::move(root)) {
}

std::unique_ptr<LibraryEntry> LibraryEntry::create(QString name, const QUuid &baseUuid, const QUuid &modificationUuid,
                                                   const QDateTime &modificationDateTime, std::set<QString> tags,
                                                   std::unique_ptr<AxiomModel::ModelRoot> root) {
    return std::make_unique<LibraryEntry>(std::move(name), baseUuid, modificationUuid, modificationDateTime,
        std::move(tags), std::move(root));
}

std::unique_ptr<LibraryEntry> LibraryEntry::create(QString name, std::set<QString> tags) {
    return create(std::move(name), QUuid::createUuid(), QUuid::createUuid(), QDateTime::currentDateTimeUtc(), std::move(tags), std::make_unique<ModelRoot>());
}

std::unique_ptr<LibraryEntry> LibraryEntry::deserialize(QDataStream &stream) {
    QString name; stream >> name;
    QUuid baseUuid; stream >> baseUuid;
    QUuid modificationUuid; stream >> modificationUuid;
    QDateTime modificationDateTime; stream >> modificationDateTime;

    quint32 tagSize; stream >> tagSize;
    std::set<QString> tags;
    for (quint32 i = 0; i < tagSize; i++) {
        QString tag; stream >> tag;
        tags.emplace(tag);
    }

    auto root = std::make_unique<ModelRoot>(stream);

    return create(std::move(name), baseUuid, modificationUuid, modificationDateTime, std::move(tags), std::move(root));
}

void LibraryEntry::serialize(QDataStream &stream) {
    stream << _name;
    stream << _baseUuid;
    stream << _modificationUuid;
    stream << _modificationDateTime;

    stream << (quint32) _tags.size();
    for (const auto &tag : _tags) {
        stream << tag;
    }

    _root->serialize(stream);
}

void LibraryEntry::setName(const QString &newName) {
    if (newName != _name) {
        _name = newName;
        nameChanged.trigger(newName);
    }
}

void LibraryEntry::addTag(const QString &tag) {
    if (_tags.insert(tag).second) tagAdded.trigger(tag);
}

void LibraryEntry::removeTag(const QString &tag) {
    if (_tags.erase(tag)) tagRemoved.trigger(tag);
}


void LibraryEntry::modified() {
    _modificationUuid = QUuid::createUuid();
    _modificationDateTime = QDateTime::currentDateTimeUtc();
}

void LibraryEntry::remove() {
    _root->destroy();
    removed.trigger();
    cleanup.trigger();
}
