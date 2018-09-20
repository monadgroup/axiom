#include "util.h"

#include <QtCore/QFile>
#include <QtCore/QStringBuilder>
#include <array>
#include <cmath>
#include <sstream>

using namespace AxiomUtil;

QString AxiomUtil::loadStylesheet(const char *path) {
    auto file = new QFile(path);
    auto couldOpen = file->open(QIODevice::ReadOnly | QIODevice::Text);
    assert(couldOpen);
    return QLatin1String(file->readAll());
}

QPoint AxiomUtil::clampP(QPoint p, QPoint min, QPoint max) {
    return QPoint(qMax(min.x(), qMin(p.x(), max.x())), qMax(min.y(), qMin(p.y(), max.y())));
}

QColor AxiomUtil::mixColor(QColor a, QColor b, float mix) {
    return QColor(a.red() + static_cast<int>((b.red() - a.red()) * mix),
                  a.green() + static_cast<int>((b.green() - a.green()) * mix),
                  a.blue() + static_cast<int>((b.blue() - a.blue()) * mix));
}

bool AxiomUtil::strToFloat(const QString &str, float &result) {
    bool ok;
    result = str.toFloat(&ok);
    return ok;
}

static std::array<QString, 12> noteNames{"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

QString AxiomUtil::getNoteName(float noteVal) {
    auto intNote = static_cast<size_t>(noteVal);
    auto noteNameIndex = intNote % noteNames.size();
    auto octave = intNote / noteNames.size();

    return QString::number(intNote) % " " % noteNames[noteNameIndex] % QString::number(octave);
}

QString AxiomUtil::formatFloatForm(float val, AxiomModel::FormType form) {
    switch (form) {
    case AxiomModel::FormType::NONE:
    case AxiomModel::FormType::CONTROL:
        return QString::number(val, 'f', 2);
    case AxiomModel::FormType::OSCILLATOR:
        return QString::number(val, 'f', 2);
    case AxiomModel::FormType::AMPLITUDE:
        return QString::number(val, 'f', 2);
    case AxiomModel::FormType::Q:
        return QString::number(val, 'f', 1);
    case AxiomModel::FormType::NOTE:
        return AxiomUtil::getNoteName(val);
    case AxiomModel::FormType::FREQUENCY:
        return val < 1000 ? QString::number(val, 'f', 2) : QString::number(val / 1000, 'f', 2);
    case AxiomModel::FormType::BEATS:
        return QString::number(val, 'f', 1);
    case AxiomModel::FormType::SECONDS:
        return val < 0.1 ? QString::number(val * 1000, 'f', 2) : QString::number(val, 'f', 2);
    case AxiomModel::FormType::SAMPLES:
        return QString::number((int) val);
    case AxiomModel::FormType::DB:
        return QString::number(val, 'f', 1);
    }
    unreachable;
}

const char *AxiomUtil::getFormUnit(float val, AxiomModel::FormType form) {
    switch (form) {
    case AxiomModel::FormType::NONE:
    case AxiomModel::FormType::CONTROL:
    case AxiomModel::FormType::NOTE:
        return "";
    case AxiomModel::FormType::OSCILLATOR:
        return "V";
    case AxiomModel::FormType::AMPLITUDE:
        return "A";
    case AxiomModel::FormType::Q:
        return "Q";
    case AxiomModel::FormType::FREQUENCY:
        return val < 1000 ? "Hz" : "KHz";
    case AxiomModel::FormType::BEATS:
        return "beats";
    case AxiomModel::FormType::SECONDS:
        return val < 0.1 ? "ms" : "s";
    case AxiomModel::FormType::SAMPLES:
        return "μ";
    case AxiomModel::FormType::DB:
        return "dB";
    }
    unreachable;
}

QString AxiomUtil::formatChannelFull(float val, AxiomModel::FormType form) {
    return formatFloatForm(val, form) % " " % getFormUnit(val, form);
}

QString AxiomUtil::formatNumForm(AxiomModel::NumValue value, bool includeForm) {
    if (fabsf(value.left - value.right) < 0.01f) {
        auto formattedNum = formatFloatForm(value.left, value.form);
        return includeForm ? static_cast<QString>(formattedNum % " " % getFormUnit(value.left, value.form))
                           : formattedNum;
    } else {
        auto formattedLeft = formatFloatForm(value.left, value.form);
        auto formattedRight = formatFloatForm(value.right, value.form);
        if (!includeForm) return formattedLeft % " / " % formattedRight;

        auto leftUnit = getFormUnit(value.left, value.form);
        auto rightUnit = getFormUnit(value.right, value.form);

        if (leftUnit == rightUnit)
            return formattedLeft % " / " % formattedRight % " " % leftUnit;
        else
            return formattedLeft % " " % leftUnit % " / " % formattedRight % " " % rightUnit;
    }
}

QRect AxiomUtil::makeRect(QPoint p1, QPoint p2) {
    QPoint topLeft;
    QPoint bottomRight;

    if (p1.x() < p2.x()) {
        topLeft.setX(p1.x());
        bottomRight.setX(p2.x());
    } else {
        topLeft.setX(p2.x());
        bottomRight.setX(p1.x());
    }

    if (p1.y() < p2.y()) {
        topLeft.setY(p1.y());
        bottomRight.setY(p2.y());
    } else {
        topLeft.setY(p2.y());
        bottomRight.setY(p1.y());
    }

    return QRect(topLeft, bottomRight);
}
