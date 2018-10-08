#include "FloatingValueEditor.h"

#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QLineEdit>

using namespace AxiomGui;

FloatingValueEditor::FloatingValueEditor(QString initialValue, QPointF scenePos, int selectStart, int selectEnd) {
    setFlag(QGraphicsItem::ItemIsFocusable, true);

    editor = new QLineEdit(initialValue);
    editor->installEventFilter(this);
    setWidget(editor);

    connect(editor, &QLineEdit::editingFinished, this, &FloatingValueEditor::editingFinished);

    if (selectStart == 0 && selectEnd == 0)
        editor->selectAll();
    else
        editor->setSelection(selectStart, selectEnd);
    editor->setFocus();

    setPos(QPointF(scenePos.x() + 30, scenePos.y()));
    setZValue(100);
}

bool FloatingValueEditor::eventFilter(QObject *object, QEvent *event) {
    if (object == editor && event->type() == QEvent::KeyPress) {
        auto keyEvent = (QKeyEvent *) event;
        if (keyEvent->key() == Qt::Key_Escape) {
            editor->clearFocus();
            editingFinished();
        }
    }

    return QGraphicsProxyWidget::eventFilter(object, event);
}

void FloatingValueEditor::editingFinished() {
    if (editor->hasFocus()) {
        emit valueSubmitted(editor->text());
    }

    scene()->removeItem(this);
}
