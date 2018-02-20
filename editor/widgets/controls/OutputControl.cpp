#include "OutputControl.h"

#include "editor/model/control/NodeOutputControl.h"
#include "../schematic/SchematicCanvas.h"

using namespace AxiomGui;

OutputControl::OutputControl(AxiomModel::NodeOutputControl *control, SchematicCanvas *canvas)
    : ControlItem(control, canvas), control(control), _image(":/icons/output.png") {

}

void OutputControl::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto size = QSizeF(SchematicCanvas::controlRealSize(control->size()));
    auto imageSize = QSizeF(_image.size());
    auto imagePos = size / 2 - imageSize / 2;

    painter->drawImage(QPointF(imagePos.width(), imagePos.height()), _image);
}

QPainterPath OutputControl::shape() const {
    auto size = QSizeF(SchematicCanvas::controlRealSize(control->size()));
    auto centerPos = QPointF(size.width() / 2, size.height() / 2);

    QPainterPath path;
    path.addEllipse(centerPos, _image.size().width() / 2, _image.size().height() / 2);
    return path;
}
