#include "CustomNodePanel.h"

#include <QtWidgets/QGraphicsProxyWidget>
#include <QtWidgets/QTextEdit>

#include "editor/model/node/CustomNode.h"
#include "editor/widgets/CommonColors.h"
#include "../schematic/SchematicCanvas.h"
#include "../ItemResizer.h"

using namespace AxiomGui;
using namespace AxiomModel;

CustomNodePanel::CustomNodePanel(CustomNode *node) : node(node) {
    connect(node, &CustomNode::beforeSizeChanged,
            this, &CustomNodePanel::triggerGeometryChange);
    connect(node, &CustomNode::sizeChanged,
            this, &CustomNodePanel::updateSize);
    connect(node, &CustomNode::panelOpenChanged,
            this, &CustomNodePanel::setOpen);
    connect(node, &CustomNode::beforePanelHeightChanged,
            this, &CustomNodePanel::triggerGeometryChange);
    connect(node, &CustomNode::panelHeightChanged,
            this, &CustomNodePanel::updateSize);

    auto resizer = new ItemResizer(ItemResizer::BOTTOM, QSizeF(0, Node::minPanelHeight));
    connect(this, &CustomNodePanel::resizerSizeChanged,
            resizer, &ItemResizer::setSize);
    connect(resizer, &ItemResizer::changed,
            this, &CustomNodePanel::resizerChanged);
    resizer->setParentItem(this);
    resizer->setZValue(0);

    textEditor = new QTextEdit();
    textProxy = new QGraphicsProxyWidget();
    textProxy->setWidget(textEditor);
    textProxy->setParentItem(this);
    textProxy->setZValue(1);

    textEditor->installEventFilter(this);
    connect(node, &CustomNode::codeChanged,
            textEditor, &QTextEdit::setPlainText);

    setOpen(node->isPanelOpen());
    updateSize();
}

QRectF CustomNodePanel::boundingRect() const {
    QRectF rect = { QPointF(0, 0), SchematicCanvas::nodeRealSize(node->size()) + QSizeF(1, node->panelHeight()) };
    return rect.marginsAdded(QMarginsF(5, 5, 5, 5));
}

void CustomNodePanel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto br = boundingRect();
    painter->setPen(QPen(CommonColors::customNodeBorder, 1));
    painter->setBrush(QBrush(CommonColors::customNodeNormal));
    painter->drawRoundedRect(br, 5, 5);
}

void CustomNodePanel::updateSize() {
    auto br = boundingRect();
    auto nodeSize = SchematicCanvas::nodeRealSize(node->size());

    textProxy->setGeometry(QRectF(
        QPointF(0, nodeSize.height() + 5),
        QSizeF(br.width() - 10, node->panelHeight() - 5)
    ));

    emit resizerSizeChanged(br.size() - QSizeF(5, 5));
}

void CustomNodePanel::setOpen(bool open) {
    setVisible(open);
}

void CustomNodePanel::triggerUpdate() {
    update();
}

void CustomNodePanel::triggerGeometryChange() {
    prepareGeometryChange();
}

void CustomNodePanel::resizerChanged(QPointF topLeft, QPointF bottomRight) {
    auto nodeSize = SchematicCanvas::nodeRealSize(node->size());
    node->setPanelHeight((float) bottomRight.y() - nodeSize.height() - 5);
}

bool CustomNodePanel::eventFilter(QObject *object, QEvent *event) {
    if (object == textEditor) {
        if (event->type() == QEvent::FocusOut) {
            node->setCode(textEditor->toPlainText());
            return true;
        } else if (event->type() == QEvent::FocusIn) {
            node->parentSurface->deselectAll();
        } else if (event->type() == QEvent::KeyPress) {
            auto keyEvent = (QKeyEvent*) event;
            if (keyEvent->key() == Qt::Key_Escape) {
                node->setPanelOpen(false);
            }
        }
    }

    return QGraphicsObject::eventFilter(object, event);
}
