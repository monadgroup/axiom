#include "DeleteObjectAction.h"

#include "../IdentityReferenceMapper.h"
#include "../ModelObject.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../serialize/ModelObjectSerializer.h"
#include "../serialize/ProjectSerializer.h"

using namespace AxiomModel;

DeleteObjectAction::DeleteObjectAction(const QUuid &uuid, QByteArray buffer, AxiomModel::ModelRoot *root)
    : Action(ActionType::DELETE_OBJECT, root), _uuid(uuid), _buffer(std::move(buffer)) {}

std::unique_ptr<DeleteObjectAction> DeleteObjectAction::create(const QUuid &uuid, QByteArray buffer,
                                                               AxiomModel::ModelRoot *root) {
    return std::make_unique<DeleteObjectAction>(uuid, std::move(buffer), root);
}

std::unique_ptr<DeleteObjectAction> DeleteObjectAction::create(const QUuid &uuid, AxiomModel::ModelRoot *root) {
    return create(uuid, QByteArray(), root);
}

void DeleteObjectAction::forward(bool, std::vector<QUuid> &compileItems) {
    auto sortedItems = collect(heapSort(getRemoveItems()));

    QDataStream stream(&_buffer, QIODevice::WriteOnly);
    ModelObjectSerializer::serializeChunk(stream, QUuid(), sortedItems);

    // We can't just iterate over a collection of ModelObjects and call remove() on them,
    // as objects also delete their children.
    // Instead, we build a list of UUIDs to delete, create a Sequence of them, and iterate
    // until that's empty.
    // We'll also need a list of parent UUIDs to build transactions later, so we can do that here.
    QSet<QUuid> usedIds;
    QSet<QUuid> parentIds;
    for (const auto &itm : sortedItems) {
        usedIds.insert(itm->uuid());
        parentIds.remove(itm->uuid());
        parentIds.insert(itm->parentUuid());
    }
    auto itemsToDelete = findAll(dynamicCast<ModelObject *>(root()->pool().sequence()), std::move(usedIds));

    // remove all items
    while (!itemsToDelete.empty()) {
        (*itemsToDelete.begin())->remove();
    }

    for (const auto &parent : parentIds) {
        compileItems.push_back(parent);
    }
}

void DeleteObjectAction::backward(std::vector<QUuid> &compileItems) {
    QDataStream stream(&_buffer, QIODevice::ReadOnly);
    IdentityReferenceMapper ref;
    auto addedObjects =
        ModelObjectSerializer::deserializeChunk(stream, ProjectSerializer::schemaVersion, root(), QUuid(), &ref);
    _buffer.clear();

    for (const auto &obj : addedObjects) {
        compileItems.push_back(obj->uuid());
        compileItems.push_back(obj->parentUuid());
    }
}

Sequence<ModelObject *> DeleteObjectAction::getLinkedItems(const QUuid &uuid) const {
    auto dependents = findDependents(dynamicCast<ModelObject *>(root()->pool().sequence()), uuid);
    auto links = flatten(map(dependents, std::function([](ModelObject *const &obj) { return obj->links(); })));
    auto linkDependents =
        flatten(map(links, std::function([this](ModelObject *const &obj) { return getLinkedItems(obj->uuid()); })));

    return distinctByUuid(flatten(std::array<Sequence<ModelObject *>, 2>{dependents, linkDependents}));
}

Sequence<ModelObject *> DeleteObjectAction::getRemoveItems() const {
    return getLinkedItems(_uuid);
}
