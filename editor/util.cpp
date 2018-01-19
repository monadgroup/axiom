#include "util.h"

#include <QtCore/QFile>

using namespace AxiomUtil;

QString AxiomUtil::loadStylesheet(const char *path) {
    auto file = new QFile(path);
    file->open(QIODevice::ReadOnly | QIODevice::Text);
    return QLatin1String(file->readAll());
}

QPoint AxiomUtil::clampP(QPoint p, QPoint min, QPoint max) {
    return QPoint(
            qMax(min.x(), qMin(p.x(), max.x())),
            qMax(min.y(), qMin(p.y(), max.y()))
    );
}

QColor AxiomUtil::mixColor(QColor a, QColor b, float mix) {
    return QColor(
            a.red() + static_cast<int>((b.red() - a.red()) * mix),
            a.green() + static_cast<int>((b.green() - a.green()) * mix),
            a.blue() + static_cast<int>((b.blue() - a.blue()) * mix)
    );
}

QPoint AxiomUtil::floorP(QPointF f) {
    return QPoint(
            (int) f.x(),
            (int) f.y()
    );
}
