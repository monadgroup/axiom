#include "util.h"

#include <QtCore/QFile>
#include <QtCore/QRegularExpression>
#include <QtCore/QSet>
#include <QtCore/QStringBuilder>
#include <QtWidgets/QMessageBox>
#include <array>
#include <cmath>
#include <sstream>

using namespace AxiomUtil;

QString AxiomUtil::loadStylesheet(const char *path) {
    QFile file(path);
    auto couldOpen = file.open(QIODevice::ReadOnly | QIODevice::Text);
    assert(couldOpen);
    return QLatin1String(file.readAll());
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
    auto intNote = static_cast<int>(noteVal);
    auto noteString = QString::number(intNote);

    if (intNote < 0) {
        return noteString;
    }

    auto noteNameIndex = intNote % noteNames.size();
    auto octave = intNote / noteNames.size();
    return noteString % " " % noteNames[noteNameIndex] % QString::number(octave);
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
    return static_cast<QString>(formatFloatForm(val, form) % " " % getFormUnit(val, form)).trimmed();
}

QString AxiomUtil::formatNumForm(AxiomModel::NumValue value, bool includeForm, int *outNumLength) {
    if (fabsf(value.left - value.right) < 0.01f) {
        auto formattedNum = formatFloatForm(value.left, value.form);
        if (outNumLength) *outNumLength = formattedNum.size();
        return includeForm ? static_cast<QString>(formattedNum % " " % getFormUnit(value.left, value.form))
                           : formattedNum;
    } else {
        auto formattedLeft = formatFloatForm(value.left, value.form);
        auto formattedRight = formatFloatForm(value.right, value.form);
        if (!includeForm) return formattedLeft % " / " % formattedRight;

        auto leftUnit = getFormUnit(value.left, value.form);
        auto rightUnit = getFormUnit(value.right, value.form);

        if (leftUnit == rightUnit) {
            auto leftSide = static_cast<QString>(formattedLeft % " / " % formattedRight);
            if (outNumLength) *outNumLength = leftSide.size();
            return leftSide % " " % leftUnit;
        } else {
            auto resultStr =
                static_cast<QString>(formattedLeft % " " % leftUnit % " / " % formattedRight % " " % rightUnit);
            if (outNumLength) *outNumLength = resultStr.size();
            return resultStr;
        }
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

std::optional<uint8_t> AxiomUtil::noteKeyToMidi(int keyCode) {
    switch (keyCode) {
    case Qt::Key_2:
        return 61;
    case Qt::Key_3:
        return 63;
    case Qt::Key_5:
        return 66;
    case Qt::Key_6:
        return 68;
    case Qt::Key_7:
        return 70;
    case Qt::Key_9:
        return 73;
    case Qt::Key_0:
        return 75;
    case Qt::Key_Q:
        return 60;
    case Qt::Key_W:
        return 62;
    case Qt::Key_E:
        return 64;
    case Qt::Key_R:
        return 65;
    case Qt::Key_T:
        return 67;
    case Qt::Key_Y:
        return 69;
    case Qt::Key_U:
        return 71;
    case Qt::Key_I:
        return 72;
    case Qt::Key_O:
        return 74;
    case Qt::Key_P:
        return 76;
    case Qt::Key_BracketLeft:
        return 77;
    case Qt::Key_BracketRight:
        return 79;
    case Qt::Key_S:
        return 49;
    case Qt::Key_D:
        return 51;
    case Qt::Key_G:
        return 54;
    case Qt::Key_H:
        return 56;
    case Qt::Key_J:
        return 58;
    case Qt::Key_L:
        return 61;
    case Qt::Key_Semicolon:
        return 63;
    case Qt::Key_Z:
        return 48;
    case Qt::Key_X:
        return 50;
    case Qt::Key_C:
        return 52;
    case Qt::Key_V:
        return 53;
    case Qt::Key_B:
        return 55;
    case Qt::Key_N:
        return 57;
    case Qt::Key_M:
        return 59;
    case Qt::Key_Comma:
        return 60;
    case Qt::Key_Period:
        return 62;
    default:
        return std::nullopt;
    }
}

int AxiomUtil::showMessageBox(QMessageBox &msgBox) {
    msgBox.setStyleSheet(loadStylesheet(":/styles/MainStyles.qss"));
    return msgBox.exec();
}

static QRegularExpression nonSafeCharacters(R"(_?[^0-9A-Za-z_]+_?)");

QString AxiomUtil::getSafeDefinition(QString definition) {
    definition.replace(nonSafeCharacters, "_");
    return definition;
}

static QRegularExpression endNumber(R"(\d+$)");

QString AxiomUtil::ensureDefinitionIsUnique(QString definition, const QSet<QString> &usedDefinitions) {
    if (usedDefinitions.find(definition) == usedDefinitions.end()) {
        return definition;
    }

    auto suffix = 2;

    // if it already ends in a number, use that for the start suffix
    auto endNumberMatch = endNumber.match(definition);
    if (endNumberMatch.hasMatch()) {
        suffix = endNumberMatch.captured(0).toInt() + 1;
        definition = definition.left(endNumberMatch.capturedStart(0));
    } else {
        definition += "_";
    }

    while (true) {
        auto combinedDef = definition + QString::number(suffix);
        if (usedDefinitions.find(combinedDef) == usedDefinitions.end()) {
            return combinedDef;
        }
    }
}
