#include "CustomNodePanel.h"

#include <QtWidgets/QGraphicsProxyWidget>

#include "../CommonColors.h"
#include "../ItemResizer.h"
#include "../surface/NodeSurfaceCanvas.h"
#include "editor/model/Project.h"
#include "editor/model/grid/GridSurface.h"
#include "editor/model/objects/CustomNode.h"

using namespace AxiomGui;
using namespace AxiomModel;

CustomNodePanel::CustomNodePanel(CustomNode *node) : node(node) {
    node->beforeSizeChanged.connect(this, &CustomNodePanel::triggerGeometryChange);
    node->sizeChanged.connect(this, &CustomNodePanel::updateSize);

    // todo: panel events, parse/compile errors, code changed event
    node->beforeSizeChanged.connect(this, &CustomNodePanel::triggerGeometryChange);
    node->sizeChanged.connect(this, &CustomNodePanel::updateSize);
    node->panelOpenChanged.connect(this, &CustomNodePanel::setOpen);
    node->beforePanelHeightChanged.connect(this, &CustomNodePanel::triggerGeometryChange);
    node->panelHeightChanged.connect(this, &CustomNodePanel::updateSize);
    node->codeChanged.connect(this, &CustomNodePanel::codeChanged);

    auto resizer = new ItemResizer(ItemResizer::BOTTOM, QSizeF(0, CustomNode::minPanelHeight));
    connect(this, &CustomNodePanel::resizerSizeChanged, resizer, &ItemResizer::setSize);
    connect(resizer, &ItemResizer::changed, this, &CustomNodePanel::resizerChanged);
    resizer->setParentItem(this);
    resizer->setZValue(0);

    textEditor = new QPlainTextEdit();
    textProxy = new QGraphicsProxyWidget();
    textProxy->setWidget(textEditor);
    textProxy->setParentItem(this);
    textProxy->setZValue(1);

    textEditor->installEventFilter(this);
    textEditor->setWordWrapMode(QTextOption::NoWrap);
    textEditor->setPlainText(node->code());
    connect(textEditor, &QPlainTextEdit::textChanged, this, &CustomNodePanel::controlTextChanged);

    codeChanged(node->code());
    setOpen(node->isPanelOpen());
    updateSize();
}

QRectF CustomNodePanel::boundingRect() const {
    QRectF rect = {QPointF(0, 0), NodeSurfaceCanvas::nodeRealSize(node->size()) + QSizeF(1, node->panelHeight())};
    return rect.marginsAdded(QMarginsF(5, 5, 5, 5));
}

void CustomNodePanel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto br = boundingRect();
    // if (showingErrors) {
    //    painter->setPen(QPen(CommonColors::errorNodeBorder, 1));
    //} else {
    //    painter->setPen(QPen(CommonColors::customNodeBorder, 1));
    //}
    painter->setPen(QPen(CommonColors::customNodeBorder, 1));
    painter->setBrush(QBrush(CommonColors::customNodeNormal));

    painter->drawRoundedRect(br, 5, 5);
}

void CustomNodePanel::updateSize() {
    auto br = boundingRect();
    auto nodeSize = NodeSurfaceCanvas::nodeRealSize(node->size());

    textProxy->setGeometry(QRectF(QPointF(0, nodeSize.height() + 5), QSizeF(br.width() - 10, node->panelHeight() - 5)));

    emit resizerSizeChanged(br.size() - QSizeF(5, 5));
}

void CustomNodePanel::setOpen(bool open) {
    setVisible(open);
}

void CustomNodePanel::clearError() {
    // hasErrors = false;
    // showingErrors = false;
    textEditor->setExtraSelections({});
    update();
}

void CustomNodePanel::codeChanged(const QString &newCode) {
    if (newCode != textEditor->toPlainText()) {
        textEditor->setPlainText(newCode);
    }
}

void CustomNodePanel::triggerUpdate() {
    update();
}

void CustomNodePanel::triggerGeometryChange() {
    prepareGeometryChange();
}

void CustomNodePanel::resizerChanged(QPointF topLeft, QPointF bottomRight) {
    auto nodeSize = NodeSurfaceCanvas::nodeRealSize(node->size());
    node->setPanelHeight((float) bottomRight.y() - nodeSize.height() - 5);
}

void CustomNodePanel::compileFinished() {
    textEditor->setPlainText(node->code());
}

bool CustomNodePanel::eventFilter(QObject *object, QEvent *event) {
    if (object == textEditor) {
        if (event->type() == QEvent::FocusOut) {
            if (beforeCode != node->code()) {
                node->doSetCodeAction(std::move(beforeCode), node->code());
                beforeCode = node->code();
            }
        } else if (event->type() == QEvent::FocusIn) {
            node->parentSurface->deselectAll();
            beforeCode = node->code();
        } else if (event->type() == QEvent::KeyPress) {
            auto keyEvent = (QKeyEvent *) event;

            if (keyEvent->key() == Qt::Key_Escape) {
                node->setPanelOpen(false);
            }
        }
    }

    return QGraphicsObject::eventFilter(object, event);
}

void CustomNodePanel::controlTextChanged() {
    node->setCode(textEditor->toPlainText());
}
