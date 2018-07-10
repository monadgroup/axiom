#include "PasteBufferAction.h"

#include "../CloneReferenceMapper.h"
#include "../IdentityReferenceMapper.h"
#include "../ModelObject.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/Node.h"
#include "../objects/NodeSurface.h"

using namespace AxiomModel;

PasteBufferAction::PasteBufferAction(const QUuid &surfaceUuid, bool isBufferFormatted, QByteArray buffer,
                                     QVector<QUuid> usedUuids, QPoint center, AxiomModel::ModelRoot *root)
    : Action(ActionType::PASTE_BUFFER, root), surfaceUuid(surfaceUuid), isBufferFormatted(isBufferFormatted),
      buffer(std::move(buffer)), usedUuids(std::move(usedUuids)), center(center) {}

std::unique_ptr<PasteBufferAction> PasteBufferAction::create(const QUuid &surfaceUuid, bool isBufferFormatted,
                                                             QByteArray buffer, QVector<QUuid> usedUuids, QPoint center,
                                                             AxiomModel::ModelRoot *root) {
    return std::make_unique<PasteBufferAction>(surfaceUuid, isBufferFormatted, std::move(buffer), std::move(usedUuids),
                                               center, root);
}

std::unique_ptr<PasteBufferAction> PasteBufferAction::create(const QUuid &surfaceUuid, QByteArray buffer, QPoint center,
                                                             AxiomModel::ModelRoot *root) {
    return create(surfaceUuid, false, std::move(buffer), QVector<QUuid>(), center, root);
}

std::unique_ptr<PasteBufferAction> PasteBufferAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid surfaceUuid;
    stream >> surfaceUuid;
    bool isBufferFormatted;
    stream >> isBufferFormatted;
    QByteArray buffer;
    stream >> buffer;
    QVector<QUuid> usedUuids;
    stream >> usedUuids;
    QPoint center;
    stream >> center;

    return create(surfaceUuid, isBufferFormatted, std::move(buffer), std::move(usedUuids), center, root);
}

void PasteBufferAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << surfaceUuid;
    stream << isBufferFormatted;
    stream << buffer;
    stream << usedUuids;
    stream << center;
}

void PasteBufferAction::forward(bool, MaximCompiler::Transaction *transaction) {
    assert(!buffer.isEmpty());
    assert(usedUuids.isEmpty());

    QDataStream stream(&buffer, QIODevice::ReadOnly);
    QPoint objectCenter;
    stream >> objectCenter;

    // deselect all nodes so we can just select the new ones
    find(root()->nodeSurfaces(), surfaceUuid)->grid().deselectAll();

    std::vector<ModelObject *> used;
    if (isBufferFormatted) {
        IdentityReferenceMapper ref;
        used = root()->deserializeChunk(stream, surfaceUuid, &ref);
    } else {
        CloneReferenceMapper ref;
        ref.setUuid(surfaceUuid, surfaceUuid);
        ref.setPos(surfaceUuid, center - objectCenter);
        used = root()->deserializeChunk(stream, surfaceUuid, &ref);
    }

    isBufferFormatted = true;
    buffer.clear();

    for (const auto &obj : used) {
        if (auto node = dynamic_cast<Node *>(obj); node && obj->parentUuid() == surfaceUuid) {
            node->select(false);
        }
        usedUuids.push_back(obj->uuid());
    }

    // build the transaction with each item and parent
    QSet<QUuid> transactionItems;
    for (const auto &obj : used) {
        if (!transactionItems.contains(obj->uuid())) {
            obj->build(transaction);
            transactionItems.insert(obj->uuid());
        }
        if (!transactionItems.contains(obj->parentUuid())) {
            find<ModelObject *>(root()->pool().sequence(), obj->parentUuid())->build(transaction);
            transactionItems.insert(obj->parentUuid());
        }
    }
}

void PasteBufferAction::backward(MaximCompiler::Transaction *transaction) {
    assert(buffer.isEmpty());
    assert(!usedUuids.isEmpty());

    QDataStream stream(&buffer, QIODevice::WriteOnly);
    QSet<QUuid> usedSet;
    for (const auto &uuid : usedUuids) {
        usedSet.insert(uuid);
    }

    auto objs = findAll(dynamicCast<ModelObject *>(root()->pool().sequence()), usedSet);
    auto collected = collect(objs);

    QSet<QUuid> parentIds;
    for (const auto &itm : collected) {
        parentIds.remove(itm->uuid());
        parentIds.insert(itm->parentUuid());
    }

    stream << QPoint(0, 0);
    ModelRoot::serializeChunk(stream, surfaceUuid, collected);

    usedUuids.clear();

    while (!objs.empty()) {
        (*objs.begin())->remove();
    }

    // build the transaction with the object parents
    auto parentItems = findAll(dynamicCast<ModelObject *>(root()->pool().sequence()), std::move(parentIds));
    for (const auto &parent : parentItems) {
        parent->build(transaction);
    }
}
