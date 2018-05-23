#include "DeleteObjectAction.h"

#include "../ModelObject.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../IdentityReferenceMapper.h"

using namespace AxiomModel;

DeleteObjectAction::DeleteObjectAction(const QUuid &uuid, QByteArray buffer, AxiomModel::ModelRoot *root)
    : Action(ActionType::DELETE_OBJECT, root), uuid(uuid), buffer(std::move(buffer)) {
}

std::unique_ptr<DeleteObjectAction> DeleteObjectAction::create(const QUuid &uuid, QByteArray buffer,
                                                               AxiomModel::ModelRoot *root) {
    return std::make_unique<DeleteObjectAction>(uuid, std::move(buffer), root);
}

std::unique_ptr<DeleteObjectAction> DeleteObjectAction::create(const QUuid &uuid, AxiomModel::ModelRoot *root) {
    return create(uuid, QByteArray(), root);
}

std::unique_ptr<DeleteObjectAction> DeleteObjectAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QByteArray buffer;
    stream >> buffer;

    return create(uuid, std::move(buffer), root);
}

void DeleteObjectAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);
    stream << uuid;
    stream << buffer;
}

bool DeleteObjectAction::forward(bool) {
    auto removeItems = getRemoveItems();
    auto needsRebuild = anyNeedRebuild(removeItems);

    auto sortedItems = heapSort(removeItems);

    QDataStream stream(&buffer, QIODevice::WriteOnly);
    ModelRoot::serializeChunk(stream, QUuid(), sortedItems);

    // We can't just iterate over a collection of ModelObjects and call remove() on them,
    // as objects also delete their children.
    // Instead, we build a list of UUIDs to delete, create a Sequence of them, and iterate
    // until that's empty.
    QSet<QUuid> usedIds;
    for (const auto &itm : sortedItems) {
        usedIds.insert(itm->uuid());
    }
    auto itemsToDelete = findAll(dynamicCast<ModelObject*>(root()->pool().sequence()), std::move(usedIds));

    // remove all items
    while (!itemsToDelete.empty()) {
        (*itemsToDelete.begin())->remove();
    }
    return needsRebuild;
}

bool DeleteObjectAction::backward() {
    QDataStream stream(&buffer, QIODevice::ReadOnly);
    IdentityReferenceMapper ref;
    root()->deserializeChunk(stream, QUuid(), &ref);
    buffer.clear();

    return anyNeedRebuild(getRemoveItems());
}

Sequence<ModelObject*> DeleteObjectAction::getLinkedItems(const QUuid &uuid) const {
    auto dependents = findDependents(dynamicCast<ModelObject *>(root()->pool().sequence()), uuid);
    auto links = flatten(map(dependents, std::function([](ModelObject *const &obj) { return obj->links(); })));
    auto linkDependents = flatten(map(links, std::function([this](ModelObject *const &obj) { return getLinkedItems(obj->uuid()); })));

    return distinctByUuid(flatten(std::array<Sequence<ModelObject*>, 2> {
        dependents,
        linkDependents
    }));
}

Sequence<ModelObject*> DeleteObjectAction::getRemoveItems() const {
    return getLinkedItems(uuid);
}

bool DeleteObjectAction::anyNeedRebuild(const Sequence<AxiomModel::ModelObject *> &sequence) const {
    for (const auto &itm : sequence) {
        if (itm->buildOnRemove()) return true;
    }
    return false;
}
