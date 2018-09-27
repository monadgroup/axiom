#pragma once

#include <QtGui/QColor>

namespace AxiomGui {

    class CommonColors {
    public:
        static const QColor customNodeNormal;
        static const QColor customNodeSelected;
        static const QColor customNodeBorder;

        static const QColor errorNodeBorder;

        static const QColor groupNodeNormal;
        static const QColor groupNodeSelected;
        static const QColor groupNodeBorder;

        static const QColor disabledNormal;
        static const QColor disabledActive;

        static const QColor numNormal;
        static const QColor numActive;

        static const QColor midiNormal;
        static const QColor midiActive;

        static const QColor exposedBorder;
    };
}
