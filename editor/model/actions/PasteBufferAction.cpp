#include "PasteBufferAction.h"

#include "../ModelRoot.h"
#include "../ModelObject.h"
#include "../PoolOperators.h"
#include "../CloneReferenceMapper.h"
#include "../IdentityReferenceMapper.h"

using namespace AxiomModel;

PasteBufferAction::PasteBufferAction(const QUuid &surfaceUuid, bool isBufferFormatted, QByteArray buffer, QVector<QUuid> usedUuids,
                                     QPoint center, AxiomModel::ModelRoot *root)
    : Action(ActionType::PASTE_BUFFER, root), surfaceUuid(surfaceUuid), isBufferFormatted(isBufferFormatted), buffer(std::move(buffer)),
    usedUuids(std::move(usedUuids)), center(center) {
}

std::unique_ptr<PasteBufferAction> PasteBufferAction::create(const QUuid &surfaceUuid, bool isBufferFormatted, QByteArray buffer,
                                                             QVector<QUuid> usedUuids, QPoint center,
                                                             AxiomModel::ModelRoot *root) {
    return std::make_unique<PasteBufferAction>(surfaceUuid, isBufferFormatted, std::move(buffer), std::move(usedUuids), center, root);
}

std::unique_ptr<PasteBufferAction> PasteBufferAction::create(const QUuid &surfaceUuid, QByteArray buffer, QPoint center,
                                                             AxiomModel::ModelRoot *root) {
    return create(surfaceUuid, false, std::move(buffer), QVector<QUuid>(), center, root);
}

std::unique_ptr<PasteBufferAction> PasteBufferAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid surfaceUuid; stream >> surfaceUuid;
    bool isBufferFormatted; stream >> isBufferFormatted;
    QByteArray buffer; stream >> buffer;
    QVector<QUuid> usedUuids; stream >> usedUuids;
    QPoint center; stream >> center;

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

bool PasteBufferAction::forward(bool) {
    assert(!buffer.isEmpty());
    assert(usedUuids.isEmpty());

    QDataStream stream(&buffer, QIODevice::ReadOnly);
    QPoint objectCenter; stream >> objectCenter;

    std::vector<ModelObject*> used;
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

    auto objs = findAll(dynamicCast<ModelObject*>(root()->pool().sequence()), usedSet);
    auto collected = collect(objs);
    auto needsBuild = false;
    for (const auto &obj : collected) {
        if (!obj->buildOnRemove()) continue;
        needsBuild = true;
        break;
    }

    stream << QPoint(0, 0);
    ModelRoot::serializeChunk(stream, surfaceUuid, collected);

    usedUuids.clear();

    while (!objs.empty()) {
        (*objs.begin())->remove();
    }

    return needsBuild;
}
