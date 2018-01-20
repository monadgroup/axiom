#pragma once

#include <QtCore/QString>
#include <QtCore/QPoint>
#include <QtGui/QColor>

namespace AxiomUtil {

    QString loadStylesheet(const char *path);

    QPoint clampP(QPoint p, QPoint min, QPoint max);

    QColor mixColor(QColor a, QColor b, float mix);

    bool strToFloat(QString str, float &result);

}
