#pragma once

#include <QtWidgets/QWidget>

#include "editor/compiler/interface/Exporter.h"

class QDoubleSpinBox;

namespace AxiomModel {
    class Project;
}

namespace AxiomGui {

    class AudioConfigWidget : public QWidget {
        Q_OBJECT

    public:
        AudioConfigWidget(const AxiomModel::Project &project);

        MaximCompiler::AudioConfig buildConfig();

    private:
        QDoubleSpinBox *sampleRateNum;
        QDoubleSpinBox *bpmNum;
    };
}
