#include "ModulePreview.h"

#include <QtGui/QPainter>

using namespace AxiomGui;

ModulePreview::ModulePreview(QWidget *parent) : QPushButton(parent) {
    setFixedSize(QSize(100, 100));
}

ModulePreview::~ModulePreview() {

}

void ModulePreview::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setPen(Qt::blue);
    painter.drawRoundedRect(QRect(10, 10, 50, 50), 5, 5);
    //QPushButton::paintEvent(event);
}
