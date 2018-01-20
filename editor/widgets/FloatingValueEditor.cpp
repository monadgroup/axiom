#include "FloatingValueEditor.h"

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QGraphicsScene>

using namespace AxiomGui;

FloatingValueEditor::FloatingValueEditor(QString initialValue, QPointF scenePos) {
    setFlag(QGraphicsItem::ItemIsFocusable, true);

    editor = new QLineEdit(initialValue);
    setWidget(editor);

    // todo: handle ESC to stop editing
    connect(editor, &QLineEdit::editingFinished,
            this, &FloatingValueEditor::editingFinished);

    editor->selectAll();
    editor->setFocus();

    setPos(QPointF(
            scenePos.x() + 30,
            scenePos.y()
    ));
    setZValue(100);
}

void FloatingValueEditor::editingFinished() {
    if (editor->hasFocus()) {
        emit valueSubmitted(editor->text());
    }

    scene()->removeItem(this);
}
