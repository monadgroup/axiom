#include "util.h"

#include <QtCore/QFile>
#include <cmath>

using namespace AxiomUtil;

QString AxiomUtil::loadStylesheet(const char *path) {
    auto file = new QFile(path);
    file->open(QIODevice::ReadOnly | QIODevice::Text);
    return QLatin1String(file->readAll());
}

