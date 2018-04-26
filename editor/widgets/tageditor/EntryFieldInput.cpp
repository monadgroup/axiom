#include "EntryFieldInput.h"

#include <QtGui/QKeyEvent>
#include <iostream>

using namespace AxiomGui;

EntryFieldInput::EntryFieldInput(QWidget *parent) : QLineEdit(parent) {
    connect(this, &EntryFieldInput::textChanged,
            this, &EntryFieldInput::resizeToFit);

    resizeToFit();
}

void EntryFieldInput::keyPressEvent(QKeyEvent *event) {
    QLineEdit::keyPressEvent(event);

    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Space || event->key() == Qt::Key_Comma)) {
        auto trimmedText = text().trimmed();
        if (!trimmedText.isEmpty()) {
            emit submitted(trimmedText);
        }
    }

    if (cursorPosition() == text().size()) {
        if (event->key() == Qt::Key_Delete) {
            emit deleteNext();
        } else if (event->key() == Qt::Key_Right) {
            emit focusNext();
        }
    }

    if (cursorPosition() == 0) {
        if (event->key() == Qt::Key_Backspace) {
            emit deletePrevious();
        } else if (event->key() == Qt::Key_Left) {
            emit focusPrevious();
        }
    }
}

void EntryFieldInput::resizeToFit() {
    auto w = fontMetrics().boundingRect(text() + "    ").width();
    resize(w, height());
}
