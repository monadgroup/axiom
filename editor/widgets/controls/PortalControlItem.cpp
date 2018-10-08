#include "PortalControlItem.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>

#include "../../util.h"
#include "../surface/NodeSurfaceCanvas.h"
#include "../surface/NodeSurfacePanel.h"
#include "../windows/MainWindow.h"
#include "editor/model/objects/ControlSurface.h"
#include "editor/model/objects/PortalControl.h"
#include "editor/model/objects/RootSurface.h"

using namespace AxiomGui;
using namespace AxiomModel;

static QString iconNameFromType(PortalControl::PortalType type, ConnectionWire::WireType wireType) {
    switch (type) {
    case PortalControl::PortalType::INPUT:
        switch (wireType) {
        case ConnectionWire::WireType::NUM:
            return "input-num-portal.png";
        case ConnectionWire::WireType::MIDI:
            return "input-midi-portal.png";
        }
        break;
    case PortalControl::PortalType::OUTPUT:
        switch (wireType) {
        case ConnectionWire::WireType::NUM:
            return "output-num-portal.png";
        case ConnectionWire::WireType::MIDI:
            return "output-midi-portal.png";
        }
        break;
    case PortalControl::PortalType::AUTOMATION:
        return "automation-portal.png";
    }
    unreachable;
}

static QString getNewPortalLabel(PortalControl *control, AxiomBackend::AudioBackend *backend) {
    auto rootSurface = dynamic_cast<RootSurface *>(control->surface()->node()->surface());
    if (!rootSurface) return "";

    auto remappedIndex = backend->internalRemapPortal(control->portalId());
    return QString::fromStdString(backend->getPortalLabel(remappedIndex));
}

PortalControlItem::PortalControlItem(AxiomModel::PortalControl *control, NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control), _image(getImagePath(control)) {
    control->labelWillChange.connect(this, &PortalControlItem::triggerUpdate);
}

void PortalControlItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    if (control->needsLabelUpdate()) {
        control->setPortalLabel(getNewPortalLabel(control, canvas->panel->window->project()->backend()));
    }

    auto size = QSizeF(NodeSurfaceCanvas::controlRealSize(control->size()));
    auto imageSize = QSizeF(_image.size());
    auto imagePos = size / 2 - imageSize / 2;

    painter->drawImage(QPointF(imagePos.width(), imagePos.height()), _image);

    QFont font;
    font.setPointSize(15);
    painter->setFont(font);
    painter->drawText(QRectF(QPointF(0, 0), size), Qt::AlignCenter, control->portalLabel());
}

QPainterPath PortalControlItem::controlPath() const {
    auto size = QSizeF(NodeSurfaceCanvas::controlRealSize(control->size()));
    auto centerPos = QPointF(size.width() / 2, size.height() / 2);

    QPainterPath path;
    path.addEllipse(centerPos, _image.size().width() / 2, _image.size().height() / 2);
    return path;
}

QString PortalControlItem::getImagePath(AxiomModel::PortalControl *control) {
    return ":/icons/" + iconNameFromType(control->portalType(), control->wireType());
}
