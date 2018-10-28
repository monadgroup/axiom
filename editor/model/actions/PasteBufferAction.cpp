#include "PasteBufferAction.h"

#include "../CloneReferenceMapper.h"
#include "../IdentityReferenceMapper.h"
#include "../ModelObject.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/Node.h"
#include "../objects/NodeSurface.h"
#include "../serialize/ModelObjectSerializer.h"
#include "../serialize/ProjectSerializer.h"

using namespace AxiomModel;

PasteBufferAction::PasteBufferAction(const QUuid &surfaceUuid, bool isBufferFormatted, QByteArray buffer,
                                     QVector<QUuid> usedUuids, QPoint center, AxiomModel::ModelRoot *root)
    : Action(ActionType::PASTE_BUFFER, root), _surfaceUuid(surfaceUuid), _isBufferFormatted(isBufferFormatted),
      _buffer(std::move(buffer)), _usedUuids(std::move(usedUuids)), _center(center) {}

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

void PasteBufferAction::forward(bool) {
    assert(!_buffer.isEmpty());
    assert(_usedUuids.isEmpty());

    QDataStream stream(&_buffer, QIODevice::ReadOnly);
    QPoint objectCenter;
    stream >> objectCenter;

    // deselect all nodes so we can just select the new ones
    find(root()->nodeSurfaces().sequence(), _surfaceUuid)->grid().deselectAll();

    std::vector<ModelObject *> used;
    if (_isBufferFormatted) {
        IdentityReferenceMapper ref;
        used = ModelObjectSerializer::deserializeChunk(stream, ProjectSerializer::schemaVersion, root(), _surfaceUuid,
                                                       &ref);
    } else {
        CloneReferenceMapper ref;
        ref.setUuid(_surfaceUuid, _surfaceUuid);
        ref.setPos(_surfaceUuid, _center - objectCenter);
        used = ModelObjectSerializer::deserializeChunk(stream, ProjectSerializer::schemaVersion, root(), _surfaceUuid,
                                                       &ref);
    }

    _isBufferFormatted = true;
    _buffer.clear();

    for (const auto &obj : used) {
        if (auto node = dynamic_cast<Node *>(obj); node && obj->parentUuid() == _surfaceUuid) {
            node->select(false);
        }
        _usedUuids.push_back(obj->uuid());
    }
}

void PasteBufferAction::backward() {
    assert(_buffer.isEmpty());
    assert(!_usedUuids.isEmpty());

    QDataStream stream(&_buffer, QIODevice::WriteOnly);
    QSet<QUuid> usedSet;
    for (const auto &uuid : _usedUuids) {
        usedSet.insert(uuid);
    }

    auto objs = findAll(AxiomCommon::dynamicCast<ModelObject *>(root()->pool().sequence().sequence()), usedSet);
    auto collected = collect(objs);

    QSet<QUuid> parentIds;
    for (const auto &itm : collected) {
        parentIds.remove(itm->uuid());
        parentIds.insert(itm->parentUuid());
    }

    stream << QPoint(0, 0);
    ModelObjectSerializer::serializeChunk(stream, _surfaceUuid, collected);

    _usedUuids.clear();

    while (!objs.empty()) {
        (*objs.begin())->remove();
    }
}
