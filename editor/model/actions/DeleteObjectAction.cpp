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
    auto sortedItems = heapSort(getLinkedItems(_uuid));

    QDataStream stream(&_buffer, QIODevice::WriteOnly);
    ModelObjectSerializer::serializeChunk(stream, QUuid(), sortedItems);

    // We can't just iterate over a collection of ModelObjects and call remove() on them,
    // as objects also delete their children.
    // Instead, we build a list of UUIDs to delete, create a Sequence of them, and iterate
    // until that's empty.
    // We'll also need a list of parent UUIDs to build transactions later, so we can do that here.
    QSet<QUuid> usedIds;
    QSet<QUuid> compileIds;
    for (const auto &itm : sortedItems) {
        usedIds.insert(itm->uuid());
        compileIds.remove(itm->uuid());

        auto compileLinks = itm->deleteCompileLinks();
        for (const auto &compileItem : compileLinks) {
            compileIds.insert(compileItem);
        }
    }
    auto itemsToDelete =
        findAll(AxiomCommon::dynamicCast<ModelObject *>(root()->pool().sequence().sequence()), std::move(usedIds));

    // remove all items
    while (!itemsToDelete.empty()) {
        (*itemsToDelete.begin())->remove();
    }

    for (const auto &compileId : compileIds) {
        compileItems.push_back(compileId);
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
    }
}

std::vector<ModelObject *> DeleteObjectAction::getLinkedItems(const QUuid &seed) const {
    auto dependents = AxiomCommon::collect(
        findDependents(AxiomCommon::dynamicCast<ModelObject *>(root()->pool().sequence().sequence()), seed));
    auto links = AxiomCommon::flatten(
        AxiomCommon::map(AxiomCommon::refSequence(&dependents), [](ModelObject *obj) { return obj->links(); }));
    auto linkDependents = AxiomCommon::collect(AxiomCommon::flatten(
        AxiomCommon::map(links, [this](ModelObject *obj) { return getLinkedItems(obj->uuid()); })));

    std::vector<ModelObject *> subSequences;
    subSequences.reserve(links.size() + linkDependents.size());

    std::back_insert_iterator<std::vector<ModelObject *>> iter(subSequences);
    std::copy(dependents.begin(), dependents.end(), iter);
    std::copy(linkDependents.begin(), linkDependents.end(), iter);

    return AxiomCommon::collect(distinctByUuid(std::move(subSequences)));
}
