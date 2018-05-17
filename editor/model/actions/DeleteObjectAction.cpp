#include "DeleteObjectAction.h"

#include "../ModelObject.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"

using namespace AxiomModel;

DeleteObjectAction::DeleteObjectAction(const QUuid &uuid, QByteArray buffer, AxiomModel::ModelRoot *root)
    : Action(ActionType::DELETE_OBJECT, root), uuid(uuid), buffer(std::move(buffer)) {
}

std::unique_ptr<DeleteObjectAction> DeleteObjectAction::create(const QUuid &uuid, QByteArray buffer,
                                                               AxiomModel::ModelRoot *root) {
    return std::make_unique<DeleteObjectAction>(uuid, std::move(buffer), root);
}

std::unique_ptr<DeleteObjectAction> DeleteObjectAction::create(const AxiomModel::ModelObject *object) {
    return create(object->uuid(), QByteArray(), object->root());
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

void DeleteObjectAction::forward(bool) {
    auto dependents = findDependents(dynamicCast<ModelObject *>(root()->pool().sequence()), uuid);

    QDataStream stream(&buffer, QIODevice::WriteOnly);
    ModelRoot::serializeChunk(stream, QUuid(), dependents);

    // this should cascade and remove all sub-dependents
    (*dependents.begin())->remove();
    assert(dependents.empty());
}

void DeleteObjectAction::backward() {
    QDataStream stream(&buffer, QIODevice::ReadOnly);
    root()->deserializeChunk(stream, QUuid());
    buffer.clear();
}
