#include "WindowResizer.h"

#include <QtGui/QMouseEvent>

using namespace AxiomGui;

WindowResizer::WindowResizer(QWidget *parent) : QWidget(parent), isDragging(false) {}

void WindowResizer::mousePressEvent(QMouseEvent *event) {
    isDragging = true;
    startMousePos = event->screenPos();
    emit startResize();
    event->accept();
}

void WindowResizer::mouseMoveEvent(QMouseEvent *event) {
    if (!isDragging) return;

    event->accept();

    auto deltaPos = event->screenPos() - startMousePos;
    emit resized(QPoint(deltaPos.x(), deltaPos.y()));
}

void WindowResizer::mouseReleaseEvent(QMouseEvent *) {
    isDragging = false;
}
