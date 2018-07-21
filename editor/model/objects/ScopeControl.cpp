#include "ScopeControl.h"

using namespace AxiomModel;

ScopeControl::ScopeControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                           QString name, bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                           ModelRoot *root)
    : Control(ControlType::SCOPE, ConnectionWire::WireType::NUM, uuid, parentUuid, pos, size, selected, std::move(name),
              showName, exposerUuid, exposingUuid, root) {}

std::unique_ptr<ScopeControl> ScopeControl::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                   bool selected, QString name, bool showName, const QUuid &exposerUuid,
                                                   const QUuid &exposingUuid, AxiomModel::ModelRoot *root) {
    return std::make_unique<ScopeControl>(uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid,
                                          exposingUuid, root);
}
