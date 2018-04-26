#include "EntryFieldItem.h"

#include <QHBoxLayout>

#include "EntryFieldBox.h"
#include "EntryFieldInput.h"

using namespace AxiomGui;

EntryFieldItem::EntryFieldItem(const QString &name, QBoxLayout *parentLayout, QWidget *parent)
    : QWidget(parent), parentLayout(parentLayout) {
    auto layout = new QHBoxLayout();
    if (!name.isNull()) {
        box = new EntryFieldBox(name, this);
        layout->addWidget(box);
    }

    input = new EntryFieldInput(this);
    layout->addWidget(input);
    setLayout(layout);

    connect(input, &EntryFieldInput::submitted,
            this, &EntryFieldItem::inputSubmitted);
    connect(input, &EntryFieldInput::deletePrevious,
            this, &EntryFieldItem::previousDeleted);
    connect(input, &EntryFieldInput::deleteNext,
            this, &EntryFieldItem::nextDeleted);
    connect(input, &EntryFieldInput::focusPrevious,
            this, &EntryFieldItem::previousFocused);
    connect(input, &EntryFieldInput::focusNext,
            this, &EntryFieldItem::nextFocused);
}

void EntryFieldItem::inputSubmitted(const QString &text) {
    input->setText("");

    auto newEntry = new EntryFieldItem(text, parentLayout, parentWidget());
    parentLayout->insertWidget(parentLayout->indexOf(this) + 1, newEntry);

    newEntry->previous = this;
    newEntry->next = next;

    if (next) next->previous = newEntry;
    next = newEntry;

    newEntry->input->setFocus(Qt::OtherFocusReason);
}

void EntryFieldItem::previousDeleted() {
    if (!previous) return;

    previous->next = next;
    previous->input->setFocus(Qt::OtherFocusReason);

    auto previousTextLength = previous->input->text().size();
    previous->input->setText(previous->input->text() + input->text());
    previous->input->setCursorPosition(previousTextLength);

    if (next) next->previous = previous;

    parentLayout->removeWidget(this);
    delete this;
}

void EntryFieldItem::nextDeleted() {
    if (!next) return;

    auto oldNext = next;
    next = oldNext->next;
    if (next) next->previous = this;

    auto previousTextLength = input->text().size();
    input->setText(input->text() + oldNext->input->text());
    input->setCursorPosition(previousTextLength);

    parentLayout->removeWidget(oldNext);
    delete oldNext;
}

void EntryFieldItem::previousFocused() {
    if (!previous) return;

    previous->input->setFocus(Qt::OtherFocusReason);
}

void EntryFieldItem::nextFocused() {
    if (!next) return;

    next->input->setFocus(Qt::OtherFocusReason);
}
