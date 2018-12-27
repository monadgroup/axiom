#include "CodeConfigWidget.h"

#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QRadioButton>

#include "editor/util.h"

using namespace AxiomGui;

CodeConfigWidget::CodeConfigWidget() {
    auto layout = new QFormLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    optimizationSelect = new QComboBox();
    optimizationSelect->addItem("None (O0)");
    optimizationSelect->addItem("Low (O1)");
    optimizationSelect->addItem("Medium (O2)");
    optimizationSelect->addItem("High (O3)");
    optimizationSelect->addItem("Min Size (Os)");
    optimizationSelect->addItem("Aggressive Size (Oz)");
    optimizationSelect->setCurrentIndex(4);
    layout->addRow("Optimization level:", optimizationSelect);

    instrumentPrefixEdit = new QLineEdit(oldSafePrefix);
    layout->addRow("Instrument prefix:", instrumentPrefixEdit);
    connect(instrumentPrefixEdit, &QLineEdit::editingFinished, this, &CodeConfigWidget::ensureInstrumentPrefixSafe);
    connect(instrumentPrefixEdit, &QLineEdit::textChanged, this, &CodeConfigWidget::processPrefixChange);

    instrumentAndLibraryContent = new QRadioButton("Instrument and library");
    instrumentContent = new QRadioButton("Just instrument");
    libraryContent = new QRadioButton("Just library");

    auto codeContentLayout = new QHBoxLayout();
    codeContentLayout->addWidget(instrumentAndLibraryContent);
    codeContentLayout->addWidget(instrumentContent);
    codeContentLayout->addWidget(libraryContent);
    codeContentLayout->addStretch(1);
    layout->addRow(codeContentLayout);

    instrumentAndLibraryContent->setChecked(true);
}

void CodeConfigWidget::processPrefixChange(const QString &newPrefix) {
    auto newSafePrefix = AxiomUtil::getSafeDefinition(newPrefix);
    if (newSafePrefix != oldSafePrefix) {
        emit instrumentPrefixChanged(oldSafePrefix, newSafePrefix);
        oldSafePrefix = newSafePrefix;
    }
}

void CodeConfigWidget::ensureInstrumentPrefixSafe() {
    auto currentText = instrumentPrefixEdit->text();
    auto safeName = AxiomUtil::getSafeDefinition(currentText);
    if (safeName != currentText) {
        instrumentPrefixEdit->setText(safeName);
    }
}
