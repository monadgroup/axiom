#pragma once

#include <QtWidgets/QWidget>

#include "editor/compiler/interface/Exporter.h"

class QComboBox;
class QSlider;

namespace AxiomGui {

    class TargetConfigWidget : public QWidget {
        Q_OBJECT

    public:
        TargetConfigWidget();

        MaximCompiler::TargetConfig buildConfig();

    private slots:
        void setToCurrentMachine();

    private:
        QComboBox *machineSelect;
        QComboBox *instructionSetSelect;
        QSlider *featureSlider;
    };
}
