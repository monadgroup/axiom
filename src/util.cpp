#include "util.h"

#include <QtCore/QFile>
#include <cmath>

using namespace AxiomUtil;

QString AxiomUtil::loadStylesheet(const char *path) {
    auto file = new QFile(path);
    file->open(QIODevice::ReadOnly | QIODevice::Text);
    return QLatin1String(file->readAll());
}

uint32_t AxiomUtil::flp2(uint32_t x) {
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >> 16);
    return x - (x >> 1);
}

double AxiomUtil::flp2(double x) {
    return std::pow(2, std::floor(std::log2(x)));
}

uint32_t AxiomUtil::clp2(uint32_t x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

double AxiomUtil::clp2(double x) {
    return std::pow(2, std::ceil(std::log2(x)));
}
