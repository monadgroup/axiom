#include "CustomNodePanel.h"

#include <QtWidgets/QGraphicsProxyWidget>

#include "../CommonColors.h"
#include "../ItemResizer.h"
#include "../surface/NodeSurfaceCanvas.h"
#include "SyntaxHighlighter.h"
#include "editor/model/Project.h"
#include "editor/model/grid/GridSurface.h"
#include "editor/model/objects/CustomNode.h"
#include "editor/util.h"

using namespace AxiomGui;
using namespace AxiomModel;

CustomNodePanel::CustomNodePanel(CustomNode *node) : node(node) {
    node->beforeSizeChanged.connectTo(this, &CustomNodePanel::triggerGeometryChange);
    node->sizeChanged.connectTo(this, &CustomNodePanel::updateSize);

    node->beforeSizeChanged.connectTo(this, &CustomNodePanel::triggerGeometryChange);
    node->sizeChanged.connectTo(this, &CustomNodePanel::updateSize);
    node->panelOpenChanged.connectTo(this, &CustomNodePanel::setOpen);
    node->beforePanelSizeChanged.connectTo(this, &CustomNodePanel::triggerGeometryChange);
    node->panelSizeChanged.connectTo(this, &CustomNodePanel::updateSize);
    node->codeChanged.connectTo(this, &CustomNodePanel::codeChanged);
    node->codeCompileError.connectTo(this, &CustomNodePanel::compileError);
    node->codeCompileSuccess.connectTo(this, &CustomNodePanel::compileSuccess);
    node->inErrorStateChanged.connectTo(this, &CustomNodePanel::triggerUpdate);

    ItemResizer::Direction resizerDirections[] = {ItemResizer::BOTTOM, ItemResizer::BOTTOM_RIGHT, ItemResizer::RIGHT};
    for (auto direction : resizerDirections) {
        auto resizer = new ItemResizer(direction, QSizeF(0, CustomNode::minPanelHeight));
        connect(this, &CustomNodePanel::resizerSizeChanged, resizer, &ItemResizer::setSize);
        connect(resizer, &ItemResizer::changed, this, &CustomNodePanel::resizerChanged);
        resizer->setParentItem(this);
        resizer->setZValue(0);
    }

    textEditor = new QPlainTextEdit();
    textEditor->setStyleSheet(AxiomUtil::loadStylesheet(":/styles/MainStyles.qss"));
    textProxy = new QGraphicsProxyWidget();
    textProxy->setWidget(textEditor);
    textProxy->setParentItem(this);
    textProxy->setZValue(1);

    textEditor->installEventFilter(this);
    textEditor->setWordWrapMode(QTextOption::NoWrap);
    textEditor->setPlainText(node->code());
    connect(textEditor, &QPlainTextEdit::textChanged, this, &CustomNodePanel::controlTextChanged);

    // highlighter automatically attaches itself to the text editor and manages its own memory
    highlighter = new SyntaxHighlighter(textEditor->document());

    codeChanged(node->code());
    setOpen(node->isPanelOpen());
    updateSize();
}

QRectF CustomNodePanel::boundingRect() const {
    QRectF rect = {QPointF(0, 0),
                   NodeSurfaceCanvas::nodeRealSize(getRealPanelSize() + QSizeF(0, node->size().height())) +
                       QSizeF(1, 0)};
    return rect.marginsAdded(QMarginsF(5, 5, 5, 10));
}

void CustomNodePanel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto br = boundingRect();
    if (node->isInErrorState()) {
        painter->setPen(QPen(CommonColors::errorNodeBorder, 1));
    } else {
        painter->setPen(QPen(CommonColors::customNodeBorder, 1));
    }

    painter->setBrush(QBrush(CommonColors::customNodeNormal));

    auto nodeSize = NodeSurfaceCanvas::nodeRealSize(node->size()) + QSizeF(11, 5);

    br = br.normalized();
    auto x = br.x();
    auto y = br.y();
    auto smallW = nodeSize.width();
    auto smallH = nodeSize.height();
    auto largeW = br.width();
    auto largeH = br.height();

    auto rx = 5.f;
    auto ry = 5.f;

    QPainterPath path;
    path.arcMoveTo(x, y, rx, ry, 180);
    path.arcTo(x, y, rx, ry, 180, -90);
    path.arcTo(x + smallW - rx, y, rx, ry, 90, -90);
    if (largeW - smallW < rx / 2) {
        path.lineTo(x + smallW, y + smallH);
        path.lineTo(x + largeW, y + smallH);
    } else {
        path.arcTo(x + smallW, y + smallH - ry, rx, ry, 180, 90);
        path.arcTo(x + largeW - rx, y + smallH, rx, ry, 90, -90);
    }
    path.arcTo(x + largeW - rx, y + largeH - ry, rx, ry, 0, -90);
    path.arcTo(x, y + largeH - ry, rx, ry, 270, -90);
    path.closeSubpath();
    painter->drawPath(path);
}

void CustomNodePanel::updateSize() {
    auto br = boundingRect();
    auto nodeSize = NodeSurfaceCanvas::nodeRealSize(node->size());

    textProxy->setGeometry(
        QRectF(QPointF(0, nodeSize.height() + 5), NodeSurfaceCanvas::nodeRealSize(getRealPanelSize()) + QSizeF(1, 0)));

    emit resizerSizeChanged(br.size() - QSizeF(5, 5));
}

void CustomNodePanel::setOpen(bool open) {
    setVisible(open);
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
    node->setPanelSize(QSizeF((bottomRight.x() - 6) / NodeSurfaceCanvas::nodeGridSize.width(),
                              (bottomRight.y() - nodeSize.height() - 10) / NodeSurfaceCanvas::nodeGridSize.height()));
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

void CustomNodePanel::compileError(const AxiomModel::CustomNodeError &error) {
    QTextCursor cursor(textEditor->document());
    moveCursor(cursor, error.sourceRange.front, QTextCursor::MoveAnchor);
    moveCursor(cursor, error.sourceRange.back, QTextCursor::KeepAnchor);

    QTextCharFormat squigglyFormat;
    squigglyFormat.setUnderlineColor(QColor::fromRgb(255, 0, 0));
    squigglyFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

    QList<QTextEdit::ExtraSelection> selections;
    selections.push_back({cursor, squigglyFormat});
    textEditor->setExtraSelections(selections);
}

void CustomNodePanel::compileSuccess() {
    textEditor->setExtraSelections({});
}

QSizeF CustomNodePanel::getRealPanelSize() const {
    return QSizeF(qMax((float) node->size().width(), (float) node->panelSize().width()), node->panelSize().height());
}

void CustomNodePanel::moveCursor(QTextCursor &cursor, MaximFrontend::SourcePos pos, QTextCursor::MoveMode mode) {
    cursor.movePosition(QTextCursor::Start, mode);
    cursor.movePosition(QTextCursor::Down, mode, pos.line);
    cursor.movePosition(QTextCursor::Right, mode, pos.column);
}
