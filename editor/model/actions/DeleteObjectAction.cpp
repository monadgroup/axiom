#include "DeleteObjectAction.h"

#include "../ModelObject.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../Pool.h"

using namespace AxiomModel;

DeleteObjectAction::DeleteObjectAction(const QUuid &uuid, const QUuid &parentUuid, QByteArray buffer, AxiomModel::ModelRoot *root)
    : Action(ActionType::DELETE_OBJECT, root), uuid(uuid), parentUuid(parentUuid), buffer(std::move(buffer)) {
}

std::unique_ptr<DeleteObjectAction> DeleteObjectAction::create(const QUuid &uuid, const QUuid &parentUuid, QByteArray buffer,
                                                               AxiomModel::ModelRoot *root) {
    return std::make_unique<DeleteObjectAction>(uuid, parentUuid, std::move(buffer), root);
}

std::unique_ptr<DeleteObjectAction> DeleteObjectAction::create(const QUuid &uuid, AxiomModel::ModelRoot *root) {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);

    auto obj = find<ModelObject*>(*root->pool(), uuid);
    obj->serialize(stream, obj->parentUuid(), false);

    return create(uuid, obj->parentUuid(), std::move(buffer), root);
}

std::unique_ptr<DeleteObjectAction> DeleteObjectAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid uuid; stream >> uuid;
    QUuid parentUuid; stream >> parentUuid;
    QByteArray buffer; stream >> buffer;

    return create(uuid, parentUuid, std::move(buffer), root);
}

void DeleteObjectAction::serialize(QDataStream &stream) const {
    stream << uuid;
    stream << parentUuid;
    stream << buffer;
}

void DeleteObjectAction::forward() const {
    find(*root()->pool(), uuid)->remove();
}

void DeleteObjectAction::backward() const {
    QDataStream stream(const_cast<QByteArray*>(&buffer), QIODevice::ReadOnly);
    root()->pool()->registerObj(ModelObject::deserialize(stream, uuid, parentUuid, root()));
}
