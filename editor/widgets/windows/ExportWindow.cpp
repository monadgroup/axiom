#include "ExportWindow.h"

#include <QtGui/QIcon>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>

#include "editor/util.h"

using namespace AxiomGui;

ExportWindow::ExportWindow()
    : QDialog(nullptr, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint) {
    setWindowTitle(tr("Export Project"));
    // setStyleSheet(AxiomUtil::loadStylesheet(":/styles/ExportWindow.qss"));
    setWindowIcon(QIcon(":/application.ico"));

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto mainLayout = new QFormLayout();

    auto instrumentTargetSection = new QGroupBox("Target");
    auto instrumentTargetLayout = new QFormLayout();
    instrumentTargetSection->setLayout(instrumentTargetLayout);
    mainLayout->addRow(instrumentTargetSection);

    auto machineSelect = new QComboBox();
    machineSelect->addItem("Windows (GNU)");
    machineSelect->addItem("Windows (MSVC)");
    machineSelect->addItem("Apple");
    machineSelect->addItem("Linux");

    auto instructionSetSelect = new QComboBox();
    instructionSetSelect->addItem("i686");
    instructionSetSelect->addItem("x86_64");

    auto machineLayout = new QHBoxLayout();
    machineLayout->addWidget(machineSelect);
    machineLayout->addWidget(instructionSetSelect);
    instrumentTargetLayout->addRow("Machine:", machineLayout);

    auto featureSupportLayout = new QHBoxLayout();
    featureSupportLayout->addWidget(new QCheckBox("SSE4.2"));
    featureSupportLayout->addWidget(new QCheckBox("AVX"));
    featureSupportLayout->addWidget(new QCheckBox("AVX2"));
    instrumentTargetLayout->addRow("Features:", featureSupportLayout);

    instrumentTargetLayout->addRow(new QPushButton("Reset to Current"));

    auto instrumentCodeSection = new QGroupBox("Code");
    auto instrumentCodeLayout = new QFormLayout();
    instrumentCodeSection->setLayout(instrumentCodeLayout);
    mainLayout->addRow(instrumentCodeSection);

    auto optimizationLayoutSelect = new QComboBox();
    optimizationLayoutSelect->addItem("None (O0)");
    optimizationLayoutSelect->addItem("Low (O1)");
    optimizationLayoutSelect->addItem("Medium (O2)");
    optimizationLayoutSelect->addItem("High (O3)");
    optimizationLayoutSelect->addItem("Min Size (Os)");
    optimizationLayoutSelect->addItem("Aggressive Size (Oz)");
    instrumentCodeLayout->addRow("Optimization level:", optimizationLayoutSelect);

    auto codeContentLayout = new QHBoxLayout();
    codeContentLayout->addWidget(new QRadioButton("Instrument and library"));
    codeContentLayout->addWidget(new QRadioButton("Just instrument"));
    codeContentLayout->addWidget(new QRadioButton("Just library"));
    instrumentCodeLayout->addRow(codeContentLayout);

    auto instrumentOutputSection = new QGroupBox("Output");
    auto instrumentOutputLayout = new QHBoxLayout();
    instrumentOutputSection->setLayout(instrumentOutputLayout);
    mainLayout->addRow(instrumentOutputSection);

    auto outputObjectSection = new QGroupBox("Object file");
    outputObjectSection->setCheckable(true);
    outputObjectSection->setChecked(true);
    auto outputObjectLayout = new QFormLayout();
    outputObjectSection->setLayout(outputObjectLayout);
    instrumentOutputLayout->addWidget(outputObjectSection);

    auto outputObjectFormatSelect = new QComboBox();
    outputObjectFormatSelect->addItem("Object file (for static linking)");
    outputObjectFormatSelect->addItem("LLVM bitcode file (for LTO)");
    outputObjectFormatSelect->addItem("LLVM IR");
    outputObjectFormatSelect->addItem("Assembly listing");
    outputObjectLayout->addRow("Format:", outputObjectFormatSelect);

    auto outputObjectLocationSelect = new QHBoxLayout();
    outputObjectLocationSelect->addWidget(new QLineEdit());
    outputObjectLocationSelect->addWidget(new QPushButton("..."));
    outputObjectLayout->addRow("Location:", outputObjectLocationSelect);

    auto outputMetaSection = new QGroupBox("Meta file");
    outputMetaSection->setCheckable(true);
    outputMetaSection->setChecked(true);
    auto outputMetaLayout = new QFormLayout();
    outputMetaSection->setLayout(outputMetaLayout);
    instrumentOutputLayout->addWidget(outputMetaSection);

    auto outputMetaFormatSelect = new QComboBox();
    outputMetaFormatSelect->addItem("Header file");
    outputMetaFormatSelect->addItem("Rust module");
    outputMetaFormatSelect->addItem("JSON file");
    outputMetaLayout->addRow("Format:", outputMetaFormatSelect);

    auto headerNameList = new QListWidget();
    auto listItem = new QListWidgetItem();

    mainLayout->addRow(new QPushButton("Export"));

    setLayout(mainLayout);
}
