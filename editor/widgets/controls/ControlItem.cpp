#include "ControlItem.h"

#include "editor/model/control/NodeValueControl.h"
#include "BasicControl.h"
#include "../ItemResizer.h"
#include "../schematic/SchematicCanvas.h"

using namespace AxiomGui;
using namespace AxiomModel;

ControlItem::ControlItem(NodeControl *control, NodeItem *parent) : control(control) {
    setHandlesChildEvents(false);

    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    // create main item
    if (auto valueControl = dynamic_cast<NodeValueControl *>(control)) {
        auto c = new BasicControl(valueControl, parent);
        c->setZValue(0);
        addToGroup(c);
    }

    connect(control, &NodeControl::posChanged,
            this, &ControlItem::setPos);
    connect(control, &NodeControl::beforeSizeChanged,
            this, &ControlItem::triggerGeometryChange);
    connect(control, &NodeControl::sizeChanged,
            this, &ControlItem::setSize);
    connect(control, &NodeControl::removed,
            this, &ControlItem::remove);

    // create resize items
    ItemResizer::Direction directions[] = {
            ItemResizer::TOP, ItemResizer::RIGHT, ItemResizer::BOTTOM, ItemResizer::LEFT,
            ItemResizer::TOP_RIGHT, ItemResizer::TOP_LEFT, ItemResizer::BOTTOM_RIGHT, ItemResizer::BOTTOM_LEFT
    };
    for (auto i = 0; i < 8; i++) {
        auto resizer = new ItemResizer(directions[i], SchematicCanvas::controlGridSize);
        resizer->enablePainting();
        resizer->setVisible(false);

        // ensure corners are on top of edges
        resizer->setZValue(i > 3 ? 3 : 2);

        connect(this, &ControlItem::resizerPosChanged,
                resizer, &ItemResizer::setPos);
        connect(this, &ControlItem::resizerSizeChanged,
                resizer, &ItemResizer::setSize);

        connect(resizer, &ItemResizer::startDrag,
                this, &ControlItem::resizerStartDrag);
        connect(resizer, &ItemResizer::changed,
                this, &ControlItem::resizerChanged);

        connect(control, &NodeControl::selected,
                resizer, [this, resizer]() { resizer->setVisible(true); });
        connect(control, &NodeControl::deselected,
                resizer, [this, resizer]() { resizer->setVisible(false); });

        addToGroup(resizer);
    }

    // set initial state
    setPos(control->pos());
    setSize(control->size());
}

void ControlItem::setPos(QPoint newPos) {
    auto realPos = SchematicCanvas::controlRealPos(newPos);
    QGraphicsItem::setPos(realPos.x(), realPos.y());
    emit resizerPosChanged(realPos);
}

void ControlItem::setSize(QSize newSize) {
    emit resizerSizeChanged(SchematicCanvas::controlRealSize(newSize));
}

void ControlItem::remove() {
    scene()->removeItem(this);
}

void ControlItem::resizerChanged(QPointF topLeft, QPointF bottomRight) {
    control->setCorners(QPoint(
            qRound(topLeft.x() / SchematicCanvas::controlGridSize.width()),
            qRound(topLeft.y() / SchematicCanvas::controlGridSize.height())
    ), QPoint(
            qRound(bottomRight.x() / SchematicCanvas::controlGridSize.width()),
            qRound(bottomRight.y() / SchematicCanvas::controlGridSize.height())
    ));
}

void ControlItem::resizerStartDrag() {
    control->select(true);
}

void ControlItem::triggerGeometryChange() {
    prepareGeometryChange();
}
