#include "IOControl.h"

#include "editor/model/control/NodeIOControl.h"
#include "editor/widgets/schematic/NodeSurfaceCanvas.h"

using namespace AxiomGui;

IOControl::IOControl(AxiomModel::NodeIOControl *control, NodeSurfaceCanvas *canvas)
    : ControlItem(control, canvas), control(control), _image(getImagePath(control)) {

}

void IOControl::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto size = QSizeF(NodeSurfaceCanvas::controlRealSize(control->size()));
    auto imageSize = QSizeF(_image.size());
    auto imagePos = size / 2 - imageSize / 2;

    painter->drawImage(QPointF(imagePos.width(), imagePos.height()), _image);
}

QPainterPath IOControl::shape() const {
    auto size = QSizeF(NodeSurfaceCanvas::controlRealSize(control->size()));
    auto centerPos = QPointF(size.width() / 2, size.height() / 2);

    QPainterPath path;
    path.addEllipse(centerPos, _image.size().width() / 2, _image.size().height() / 2);
    return path;
}

QString IOControl::getImagePath(AxiomModel::NodeIOControl *control) {
    return QString::fromStdString(":/icons/" + MaximCommon::controlType2String(control->ioType()) + "-io.png");
}
