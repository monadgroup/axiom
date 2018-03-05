#include "CustomNodePanel.h"

#include <QtWidgets/QGraphicsProxyWidget>

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
    connect(node, &CustomNode::compileFailed,
            this, &CustomNodePanel::setError);
    connect(node, &CustomNode::parseFailed,
            this, &CustomNodePanel::setError);
    connect(node, &CustomNode::compileSucceeded,
            this, &CustomNodePanel::clearError);
    connect(node, &CustomNode::parseSucceeded,
            this, &CustomNodePanel::clearError);

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
    connect(textEditor, &QTextEdit::textChanged,
            this, &CustomNodePanel::controlTextChanged);

    setOpen(node->isPanelOpen());
    updateSize();
}

QRectF CustomNodePanel::boundingRect() const {
    QRectF rect = {QPointF(0, 0), SchematicCanvas::nodeRealSize(node->size()) + QSizeF(1, node->panelHeight())};
    return rect.marginsAdded(QMarginsF(5, 5, 5, 5));
}

void CustomNodePanel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto br = boundingRect();
    if (showingErrors) {
        painter->setPen(QPen(CommonColors::errorNodeBorder, 1));
    } else {
        painter->setPen(QPen(CommonColors::customNodeBorder, 1));
    }
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

void CustomNodePanel::setError(const MaximRuntime::ErrorLog &log) {
    hasErrors = true;
    QList<QTextEdit::ExtraSelection> selections;
    for (const auto &err : log.errors) {
        QTextCursor cursor(textEditor->document());
        moveCursor(cursor, err.start, QTextCursor::MoveAnchor);
        moveCursor(cursor, err.end, QTextCursor::KeepAnchor);

        QTextCharFormat squigglyFormat;
        squigglyFormat.setUnderlineColor(QColor::fromRgb(255, 0, 0));
        squigglyFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

        selections.push_back({cursor, squigglyFormat});
    }
    textEditor->setExtraSelections(selections);
    update();
}

void CustomNodePanel::clearError() {
    hasErrors = false;
    showingErrors = false;
    textEditor->setExtraSelections({});
    update();
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
            showingErrors = hasErrors;
            update();
            node->recompile();
            return true;
        } else if (event->type() == QEvent::FocusIn) {
            node->parentSurface->deselectAll();
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

void CustomNodePanel::moveCursor(QTextCursor &cursor, SourcePos pos, QTextCursor::MoveMode mode) {
    cursor.movePosition(QTextCursor::Start, mode);
    cursor.movePosition(QTextCursor::Down, mode, pos.line);
    cursor.movePosition(QTextCursor::Right, mode, pos.column);
}
