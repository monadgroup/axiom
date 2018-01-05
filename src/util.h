#pragma once

#include <QtCore/QString>
#include <QtCore/QPoint>

namespace AxiomUtil {

    QString loadStylesheet(const char *path);

    QPoint clampP(QPoint p, QPoint min, QPoint max);

}
