#pragma once

#include <QtWidgets/QWidget>

class QDoubleSpinBox;

namespace AxiomModel {
    class Project;
}

namespace AxiomGui {

    class AudioConfigWidget : public QWidget {
        Q_OBJECT

    public:
        AudioConfigWidget(const AxiomModel::Project &project);

    private:
        QDoubleSpinBox *sampleRateNum;
        QDoubleSpinBox *bpmNum;
    };
}
