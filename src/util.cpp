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
