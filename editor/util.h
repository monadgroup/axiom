#pragma once

#include <QtCore/QString>
#include <QtCore/QPoint>
#include <QtGui/QColor>
#include <cassert>

#define unreachable assert(false); throw

namespace AxiomUtil {

    QString loadStylesheet(const char *path);

    QPoint clampP(QPoint p, QPoint min, QPoint max);

    QColor mixColor(QColor a, QColor b, float mix);

    bool strToFloat(QString str, float &result);

    template<class InputIterator, class T>
    InputIterator findUnique(InputIterator first, InputIterator last, const T *ptr) {
        while (first != last) {
            if (first->get() == ptr) return first;
            ++first;
        }
        return last;
    };

}
