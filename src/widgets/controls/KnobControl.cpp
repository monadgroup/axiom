#include "KnobControl.h"

#include <QtGui/QPainter>

using namespace AxiomGui;

void KnobControl::paintEvent(QPaintEvent *p) {
    QDial::paintEvent(p);
}
