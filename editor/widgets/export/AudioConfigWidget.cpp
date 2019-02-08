#include "AudioConfigWidget.h"

#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <cfloat>

#include "editor/compiler/interface/Runtime.h"
#include "editor/model/ModelRoot.h"
#include "editor/model/Project.h"

using namespace AxiomGui;

AudioConfigWidget::AudioConfigWidget(const AxiomModel::Project &project) {
    auto layout = new QFormLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    sampleRateNum = new QDoubleSpinBox();
    sampleRateNum->setMinimum(0.1);
    sampleRateNum->setMaximum(DBL_MAX);
    sampleRateNum->setValue(project.mainRoot().runtime()->getSampleRate());
    sampleRateNum->setSuffix(" Hz");
    layout->addRow("Sample rate:", sampleRateNum);

    bpmNum = new QDoubleSpinBox();
    bpmNum->setMinimum(0.1);
    bpmNum->setMaximum(DBL_MAX);
    bpmNum->setValue(project.mainRoot().runtime()->getBpm());
    layout->addRow("BPM:", bpmNum);
}

MaximCompiler::AudioConfig AudioConfigWidget::buildConfig() {
    auto sampleRate = sampleRateNum->value();
    auto bpm = bpmNum->value();

    return MaximCompiler::AudioConfig(sampleRate, bpm);
}
