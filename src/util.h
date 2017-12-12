#pragma once
#include <QtCore/QString>

namespace AxiomUtil {

    QString loadStylesheet(const char *path);

    uint32_t flp2(uint32_t x);
    double flp2(double x);
    uint32_t clp2(uint32_t x);
    double clp2(double x);
}
