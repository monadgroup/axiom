#include "LibraryEntry.h"

#include "PoolOperators.h"
#include "objects/ModuleSurface.h"

using namespace AxiomModel;

LibraryEntry::LibraryEntry(QString name, const QUuid &baseUuid, const QUuid &modificationUuid,
                           const QDateTime &modificationDateTime, std::set<QString> tags,
                           std::unique_ptr<AxiomModel::ModelRoot> root)
    : _name(std::move(name)), _baseUuid(baseUuid), _modificationUuid(modificationUuid),
      _modificationDateTime(modificationDateTime), _tags(std::move(tags)), _root(std::move(root)) {
    auto rootSurfaces = findChildren(_root->nodeSurfaces().sequence(), QUuid());
    assert(rootSurfaces.size() == 1);
    _rootSurface = dynamic_cast<ModuleSurface *>(*AxiomCommon::takeAt(rootSurfaces, 0));
    assert(_rootSurface);
    _rootSurface->setEntry(this);

    _root->history().stackChanged.connect(this, &LibraryEntry::modified);
}

std::unique_ptr<LibraryEntry> LibraryEntry::create(QString name, const QUuid &baseUuid, const QUuid &modificationUuid,
                                                   const QDateTime &modificationDateTime, std::set<QString> tags,
                                                   std::unique_ptr<AxiomModel::ModelRoot> root) {
    return std::make_unique<LibraryEntry>(std::move(name), baseUuid, modificationUuid, modificationDateTime,
                                          std::move(tags), std::move(root));
}

std::unique_ptr<LibraryEntry> LibraryEntry::create(QString name, std::set<QString> tags) {
    auto newRoot = std::make_unique<ModelRoot>();
    newRoot->pool().registerObj(std::make_unique<ModuleSurface>(QUuid::createUuid(), QPointF(0, 0), 0, newRoot.get()));
    return create(std::move(name), QUuid::createUuid(), QUuid::createUuid(), QDateTime::currentDateTimeUtc(),
                  std::move(tags), std::move(newRoot));
}

void LibraryEntry::setName(const QString &newName) {
    if (newName != _name) {
        _name = newName;
        nameChanged(newName);
        modified();
    }
}

void LibraryEntry::setBaseUuid(QUuid newUuid) {
    _baseUuid = newUuid;
}

void LibraryEntry::addTag(const QString &tag) {
    if (_tags.insert(tag).second) {
        tagAdded(tag);
        modified();
    }
}

void LibraryEntry::removeTag(const QString &tag) {
    if (_tags.erase(tag)) {
        tagRemoved(tag);
        modified();
    }
}

void LibraryEntry::modified() {
    _modificationUuid = QUuid::createUuid();
    _modificationDateTime = QDateTime::currentDateTimeUtc();
    changed();
}

void LibraryEntry::remove() {
    _root->destroy();
    removed();
    cleanup();
}
