#pragma once

#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QString>
#include <QtGui/QColor>
#include <cassert>
#include <optional>

#include "model/Value.h"

#define unreachable \
    { \
        assert(false); \
        abort(); \
    }

namespace AxiomUtil {

    QString loadStylesheet(const char *path);

    QPoint clampP(QPoint p, QPoint min, QPoint max);

    QColor mixColor(QColor a, QColor b, float mix);

    bool strToFloat(const QString &str, float &result);

    QString getNoteName(float noteVal);

    QString formatFloatForm(float val, AxiomModel::FormType form);

    const char *getFormUnit(float val, AxiomModel::FormType form);

    QString formatChannelFull(float val, AxiomModel::FormType form);

    QString formatNumForm(AxiomModel::NumValue value, bool includeForm);

    QRect makeRect(QPoint p1, QPoint p2);

    std::optional<uint8_t> noteKeyToMidi(int keyCode);

    template<class InputIterator, class T>
    InputIterator findUnique(InputIterator first, InputIterator last, const T *ptr) {
        while (first != last) {
            if (first->get() == ptr) return first;
            ++first;
        }
        return last;
    };
}
