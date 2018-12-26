#pragma once

#include <QtWidgets/QWidget>

class QComboBox;
class QSlider;

namespace AxiomGui {

    class TargetConfigWidget : public QWidget {
        Q_OBJECT

    public:
        TargetConfigWidget();

    private slots:
        void setToCurrentMachine();

    private:
        QComboBox *machineSelect;
        QComboBox *instructionSetSelect;
        QSlider *featureSlider;
    };
}
