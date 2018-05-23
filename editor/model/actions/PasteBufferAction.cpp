#include "PasteBufferAction.h"

#include "../ModelRoot.h"
#include "../ModelObject.h"
#include "../PoolOperators.h"
#include "../CloneReferenceMapper.h"

using namespace AxiomModel;

PasteBufferAction::PasteBufferAction(const QUuid &surfaceUuid, QByteArray buffer, QVector<QUuid> usedUuids,
                                     QPoint center, AxiomModel::ModelRoot *root)
    : Action(ActionType::PASTE_BUFFER, root), surfaceUuid(surfaceUuid), buffer(std::move(buffer)), center(center) {
}

std::unique_ptr<PasteBufferAction> PasteBufferAction::create(const QUuid &surfaceUuid, QByteArray buffer,
                                                             QVector<QUuid> usedUuids, QPoint center,
                                                             AxiomModel::ModelRoot *root) {
    return std::make_unique<PasteBufferAction>(surfaceUuid, std::move(buffer), std::move(usedUuids), center, root);
}

std::unique_ptr<PasteBufferAction> PasteBufferAction::create(const QUuid &surfaceUuid, QByteArray buffer, QPoint center,
                                                             AxiomModel::ModelRoot *root) {
    return create(surfaceUuid, std::move(buffer), QVector<QUuid>(), center, root);
}

std::unique_ptr<PasteBufferAction> PasteBufferAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid surfaceUuid; stream >> surfaceUuid;
    QByteArray buffer; stream >> buffer;
    QVector<QUuid> usedUuids; stream >> usedUuids;
    QPoint center; stream >> center;

    return create(surfaceUuid, std::move(buffer), std::move(usedUuids), center, root);
}

void PasteBufferAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << surfaceUuid;
    stream << buffer;
    stream << usedUuids;
    stream << center;
}

bool PasteBufferAction::forward(bool) {
    assert(!buffer.isEmpty());
    assert(usedUuids.isEmpty());

    QDataStream stream(&buffer, QIODevice::ReadOnly);
    CloneReferenceMapper ref;
    ref.set(surfaceUuid, surfaceUuid);
    auto used = root()->deserializeChunk(stream, surfaceUuid, &ref);

    buffer.clear();

    auto needsBuild = false;
    for (const auto &obj : used) {
        usedUuids.push_back(obj->uuid());
        if (obj->buildOnRemove()) needsBuild = true;
    }

    return needsBuild;
}

bool PasteBufferAction::backward() {
    assert(buffer.isEmpty());
    assert(!usedUuids.isEmpty());

    QDataStream stream(&buffer, QIODevice::WriteOnly);
    QSet<QUuid> usedSet;
    for (const auto &uuid : usedUuids) {
        usedSet.insert(uuid);
    }

    auto objs = collect(findAll(dynamicCast<ModelObject*>(root()->pool().sequence()), usedSet));
    auto needsBuild = false;
    for (const auto &obj : objs) {
        if (!obj->buildOnRemove()) continue;
        needsBuild = true;
        break;
    }

    ModelRoot::serializeChunk(stream, surfaceUuid, objs);
    usedUuids.clear();

    return needsBuild;
}
