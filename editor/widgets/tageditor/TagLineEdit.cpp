#include "TagLineEdit.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QHBoxLayout>
#include <QtGui/QKeyEvent>

#include "EntryFieldItem.h"
#include "EntryFieldInput.h"
#include "editor/util.h"

using namespace AxiomGui;

TagLineEdit::TagLineEdit(QWidget *parent) : QScrollArea(parent) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/TagLineEdit.qss"));

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    layout = new QHBoxLayout();
    firstItem = new EntryFieldItem(QString::null, layout, this);
    layout->addWidget(firstItem);
    setLayout(layout);
}

void TagLineEdit::setCompleter(QCompleter *completer) {
    EntryFieldItem *currentItem = firstItem;
    while (currentItem != nullptr) {
        currentItem->input->setCompleter(completer);
        currentItem = currentItem->next;
    }
}

void TagLineEdit::addTag(const QString &name) {

}

void TagLineEdit::removeTag(const QString &name) {

}
