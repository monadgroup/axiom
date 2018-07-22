#include "GraphControl.h"

using namespace AxiomModel;

GraphControl::GraphControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                           QString name, bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                           AxiomModel::ModelRoot *root)
    : Control(ControlType::GRAPH, ConnectionWire::WireType::NUM, uuid, parentUuid, pos, size, selected, std::move(name),
              showName, exposerUuid, exposingUuid, root) {}

std::unique_ptr<GraphControl> GraphControl::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                   bool selected, QString name, bool showName, const QUuid &exposerUuid,
                                                   const QUuid &exposingUuid, AxiomModel::ModelRoot *root) {
    return std::make_unique<GraphControl>(uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid,
                                          exposingUuid, root);
}

void GraphControl::doRuntimeUpdate() {}

void GraphControl::setZoom(float zoom) {
    if (zoom != _zoom) {
        _zoom = zoom;
        zoomChanged.trigger(zoom);
    }
}
