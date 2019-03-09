#include "LibraryEntry.h"

#include "PoolOperators.h"
#include "objects/ModuleSurface.h"

using namespace AxiomModel;

LibraryEntry::LibraryEntry(QString name, const QUuid &baseUuid, const QUuid &modificationUuid,
                           const QDateTime &modificationDateTime, bool isBuiltin, std::set<QString> tags,
                           std::unique_ptr<AxiomModel::ModelRoot> root)
    : _name(std::move(name)), _baseUuid(baseUuid), _modificationUuid(modificationUuid),
      _modificationDateTime(modificationDateTime), _isBuiltin(isBuiltin), _tags(std::move(tags)),
      _root(std::move(root)) {
    auto rootSurfaces = findChildren(_root->nodeSurfaces().sequence(), QUuid());
    assert(rootSurfaces.size() == 1);
    _rootSurface = dynamic_cast<ModuleSurface *>(*AxiomCommon::takeAt(rootSurfaces, 0));
    assert(_rootSurface);
    _rootSurface->setEntry(this);

    _root->history().stackChanged.connectTo(this, &LibraryEntry::modified);
}

std::unique_ptr<LibraryEntry> LibraryEntry::create(QString name, const QUuid &baseUuid, const QUuid &modificationUuid,
                                                   const QDateTime &modificationDateTime, bool isBuiltin,
                                                   std::set<QString> tags,
                                                   std::unique_ptr<AxiomModel::ModelRoot> root) {
    return std::make_unique<LibraryEntry>(std::move(name), baseUuid, modificationUuid, modificationDateTime, isBuiltin,
                                          std::move(tags), std::move(root));
}

std::unique_ptr<LibraryEntry> LibraryEntry::create(QString name, std::set<QString> tags) {
    auto newRoot = std::make_unique<ModelRoot>();
    newRoot->pool().registerObj(std::make_unique<ModuleSurface>(QUuid::createUuid(), QPointF(0, 0), 0, newRoot.get()));
    return create(std::move(name), QUuid::createUuid(), QUuid::createUuid(), QDateTime::currentDateTimeUtc(), false,
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

void LibraryEntry::setTags(const std::set<QString> &newTags) {
    // remove old tags
    std::set<QString> oldTags(tags());
    for (const auto &oldTag : oldTags) {
        if (newTags.find(oldTag) == newTags.end()) removeTag(oldTag);
    }

    // add new tags
    for (const auto &newTag : newTags) {
        addTag(newTag);
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
