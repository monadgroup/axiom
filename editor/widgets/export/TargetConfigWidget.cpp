#include "TargetConfigWidget.h"

#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QVBoxLayout>

#include "editor/compiler/interface/Frontend.h"

using namespace AxiomGui;

TargetConfigWidget::TargetConfigWidget() {
    auto layout = new QFormLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    machineSelect = new QComboBox();
    machineSelect->addItem("Windows (MSVC)");
    machineSelect->addItem("Windows (GNU)");
    machineSelect->addItem("Apple");
    machineSelect->addItem("Linux");

    instructionSetSelect = new QComboBox();
    instructionSetSelect->addItem("i686 (32-bit)");
    instructionSetSelect->addItem("x86_64 (64-bit)");

    auto machineLayout = new QHBoxLayout();
    machineLayout->addWidget(machineSelect);
    machineLayout->addWidget(instructionSetSelect);
    layout->addRow("Machine:", machineLayout);

    featureSlider = new QSlider(Qt::Horizontal);
    featureSlider->setRange(0, 3);
    featureSlider->setSingleStep(1);
    auto sliderLayout = new QVBoxLayout();
    sliderLayout->addWidget(featureSlider);

    auto labelsLayout = new QHBoxLayout();
    labelsLayout->addWidget(new QLabel("SSE4.1"), 1);
    auto sse42Label = new QLabel("SSE4.2");
    sse42Label->setAlignment(Qt::AlignHCenter);
    labelsLayout->addWidget(sse42Label, 2);
    auto avxLabel = new QLabel("AVX");
    avxLabel->setAlignment(Qt::AlignHCenter);
    labelsLayout->addWidget(avxLabel, 2);
    auto avx2Label = new QLabel("AVX2");
    avx2Label->setAlignment(Qt::AlignRight);
    labelsLayout->addWidget(avx2Label, 1);

    /*auto rightLayout = new QHBoxLayout();
    rightLayout->addWidget(new QLabel("AVX"), 2);
    auto avx2Label = new QLabel("AVX2");
    avx2Label->setAlignment(Qt::AlignRight);
    rightLayout->addWidget(avx2Label, 1);
    labelsLayout->addLayout(rightLayout);*/
    sliderLayout->addLayout(labelsLayout);

    /*auto sliderLayout = new QGridLayout();
    sliderLayout->addWidget(featureSlider, 0, 0);
    sliderLayout->addWidget(new QLabel("SSE4.1"), 1, 0);
    sliderLayout->addWidget(new QLabel("SSE4.2"), 1, 1);
    auto rightLayout = new QHBoxLayout();
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->addWidget(new QLabel("AVX"));
    rightLayout->addWidget(new QLabel("AVX2"), Qt::AlignRight);
    sliderLayout->addLayout(rightLayout, 1, 2);*/
    /*sliderLayout->addWidget(new QLabel("AVX"), 1, 2);
    auto avx2Label = new QLabel("AVX2");
    avx2Label->setAlignment(Qt::AlignRight);
    sliderLayout->addWidget(avx2Label, 1, 2);*/
    layout->addRow("Features:", sliderLayout);

    auto resetButton = new QPushButton("Reset to Current");
    resetButton->setFixedWidth(100);
    auto resetButtonLayout = new QHBoxLayout();
    resetButtonLayout->addWidget(resetButton, 0, Qt::AlignRight);
    layout->addRow(resetButtonLayout);
    connect(resetButton, &QPushButton::clicked, this, &TargetConfigWidget::setToCurrentMachine);

    setToCurrentMachine();
}

void TargetConfigWidget::setToCurrentMachine() {
#ifdef Q_OS_WIN
    machineSelect->setCurrentIndex(0);
#elif defined(Q_OS_DARWIN)
    machineSelect->setCurrentIndex(2);
#elif defined(Q_OS_UNIX)
    machineSelect->setCurrentIndex(3);
#endif

    // in general we use i686 export more often, so always select that by default
    instructionSetSelect->setCurrentIndex(0);

    auto featureLevel = MaximFrontend::maxim_get_feature_level();
    featureSlider->setValue((int) featureLevel);
}
