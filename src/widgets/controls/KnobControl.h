#pragma once

#include <QtWidgets/QDial>

namespace AxiomGui {

    class KnobControl : public QDial {
        Q_OBJECT
    public:


    protected:
        void paintEvent(QPaintEvent *p) override;
    };

}
