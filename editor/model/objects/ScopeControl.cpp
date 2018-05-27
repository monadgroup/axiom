#include "ScopeControl.h"

#include "compiler/runtime/Control.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

ScopeControl::ScopeControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                           QString name, bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                           ModelRoot *root)
    : Control(ControlType::SCOPE, ConnectionWire::WireType::NUM, uuid, parentUuid, pos, size, selected, std::move(name),
              showName, exposerUuid, exposingUuid, root) {
}

std::unique_ptr<ScopeControl> ScopeControl::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                   bool selected, QString name, bool showName, const QUuid &exposerUuid,
                                                   const QUuid &exposingUuid, AxiomModel::ModelRoot *root) {
    return std::make_unique<ScopeControl>(uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid,
                                          exposingUuid, root);
}

std::unique_ptr<ScopeControl> ScopeControl::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                        QPoint pos, QSize size, bool selected, QString name,
                                                        bool showName, const QUuid &exposerUuid,
                                                        const QUuid &exposingUuid, AxiomModel::ReferenceMapper *ref,
                                                        AxiomModel::ModelRoot *root) {
    return create(uuid, parentUuid, pos, size, selected, name, showName, exposerUuid, exposingUuid, root);
}

void ScopeControl::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    Control::serialize(stream, parent, withContext);
}

void ScopeControl::doRuntimeUpdate() {
    if (runtime() && (*runtime())->currentPtr()) {
        auto run = *runtime();
        auto controlData = run->runtime()->op().controlData(run->currentPtr());
        auto currentPos = run->runtime()->op().readScopePos(controlData);
        std::cout << "Current pos = " << currentPos << std::endl;
    }

    // todo
}

void ScopeControl::saveValue() {
    // todo
}

void ScopeControl::restoreValue() {
    // todo
}
