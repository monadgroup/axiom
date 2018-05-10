#include "PortalControlItem.h"

#include "editor/model/objects/PortalControl.h"
#include "../surface/NodeSurfaceCanvas.h"
#include "../../util.h"

using namespace AxiomGui;
using namespace AxiomModel;

QString iconNameFromType(PortalControl::PortalType type, ConnectionWire::WireType wireType) {
    switch (type) {
        case PortalControl::PortalType::INPUT:
            switch (wireType) {
                case ConnectionWire::WireType::NUM: return "input-num-portal.png";
                case ConnectionWire::WireType::MIDI: return "input-midi-portal.png";
            }
            break;
        case PortalControl::PortalType::OUTPUT:
            switch (wireType) {
                case ConnectionWire::WireType::NUM: return "output-num-portal.png";
                case ConnectionWire::WireType::MIDI: return "output-midi-portal.png";
            }
            break;
        case PortalControl::PortalType::AUTOMATION: return "automation-portal.png";
    }
    unreachable;
}

PortalControlItem::PortalControlItem(AxiomModel::PortalControl *control, NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control), _image(getImagePath(control)) {

}

void PortalControlItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto size = QSizeF(NodeSurfaceCanvas::controlRealSize(control->size()));
    auto imageSize = QSizeF(_image.size());
    auto imagePos = size / 2 - imageSize / 2;

    painter->drawImage(QPointF(imagePos.width(), imagePos.height()), _image);
}

QPainterPath PortalControlItem::shape() const {
    auto size = QSizeF(NodeSurfaceCanvas::controlRealSize(control->size()));
    auto centerPos = QPointF(size.width() / 2, size.height() / 2);

    QPainterPath path;
    path.addEllipse(centerPos, _image.size().width() / 2, _image.size().height() / 2);
    return path;
}

QString PortalControlItem::getImagePath(AxiomModel::PortalControl *control) {
    return ":/icons/" + iconNameFromType(control->portalType(), control->wireType());
}
