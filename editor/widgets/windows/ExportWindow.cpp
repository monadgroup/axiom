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
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>

#include "../export/AudioConfigWidget.h"
#include "../export/CodeConfigWidget.h"
#include "../export/MetaOutputConfigWidget.h"
#include "../export/ObjectOutputConfigWidget.h"
#include "../export/TargetConfigWidget.h"
#include "editor/model/Project.h"
#include "editor/model/objects/RootSurface.h"
#include "editor/util.h"

using namespace AxiomGui;

ExportWindow::ExportWindow(const AxiomModel::Project &project)
    : QDialog(nullptr, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint), project(project) {
    setWindowTitle(tr("Export Project"));
    setStyleSheet(AxiomUtil::loadStylesheet(":/styles/UnstyledWindow.qss"));
    setWindowIcon(QIcon(":/application.ico"));

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    audioConfigWidget = new AudioConfigWidget(project);
    targetConfigWidget = new TargetConfigWidget();
    codeConfigWidget = new CodeConfigWidget();
    objectOutputConfigWidget = new ObjectOutputConfigWidget();
    auto audioConfiguration = project.getAudioConfiguration();
    std::sort(audioConfiguration.portals.begin(), audioConfiguration.portals.end());
    metaOutputConfigWidget = new MetaOutputConfigWidget(project, audioConfiguration);

    auto mainLayout = new QFormLayout();

    auto audioSection = new QGroupBox("Audio");
    auto audioLayout = new QGridLayout();
    audioLayout->addWidget(audioConfigWidget, 0, 0);
    audioSection->setLayout(audioLayout);
    mainLayout->addRow(audioSection);

    auto targetSection = new QGroupBox("Target");
    auto targetLayout = new QGridLayout();
    targetLayout->addWidget(targetConfigWidget, 0, 0);
    targetSection->setLayout(targetLayout);
    mainLayout->addRow(targetSection);

    auto codeSection = new QGroupBox("Code");
    auto codeLayout = new QGridLayout();
    codeLayout->addWidget(codeConfigWidget, 0, 0);
    codeSection->setLayout(codeLayout);
    mainLayout->addRow(codeSection);

    auto instrumentOutputSection = new QGroupBox("Output");
    auto instrumentOutputLayout = new QHBoxLayout();
    instrumentOutputSection->setLayout(instrumentOutputLayout);
    mainLayout->addRow(instrumentOutputSection);

    outputObjectSection = new QGroupBox("Object file");
    outputObjectSection->setCheckable(true);
    outputObjectSection->setChecked(true);
    auto outputObjectLayout = new QGridLayout();
    outputObjectLayout->addWidget(objectOutputConfigWidget, 0, 0);
    outputObjectSection->setLayout(outputObjectLayout);
    instrumentOutputLayout->addWidget(outputObjectSection);

    outputMetaSection = new QGroupBox("Meta file");
    outputMetaSection->setCheckable(true);
    outputMetaSection->setChecked(true);
    auto outputMetaLayout = new QGridLayout();
    outputMetaLayout->addWidget(metaOutputConfigWidget, 0, 0);
    outputMetaSection->setLayout(outputMetaLayout);
    instrumentOutputLayout->addWidget(outputMetaSection);

    auto actionButtonsLayout = new QHBoxLayout();
    actionButtonsLayout->addStretch(1);
    auto cancelButton = new QPushButton("Cancel");
    actionButtonsLayout->addWidget(cancelButton);
    auto exportButton = new QPushButton("Export");
    actionButtonsLayout->addWidget(exportButton);
    exportButton->setDefault(true);

    mainLayout->addRow(actionButtonsLayout);
    setLayout(mainLayout);

    connect(codeConfigWidget, &CodeConfigWidget::instrumentPrefixChanged, metaOutputConfigWidget,
            &MetaOutputConfigWidget::setInstrumentPrefix);
    connect(cancelButton, &QPushButton::clicked, this, &ExportWindow::close);
    connect(exportButton, &QPushButton::clicked, this, &ExportWindow::doExport);
}

void ExportWindow::doExport() {
    auto config = buildConfig();
    setEnabled(false);

    // build a transaction from the project
    MaximCompiler::Transaction transaction;
    project.rootSurface()->buildAll(&transaction);

    // run the exporter
    MaximCompiler::Exporter::exportTransaction(config, std::move(transaction));

    QMessageBox::information(this, "Export Complete", "The export has been completed.");
    setEnabled(true);
}

MaximCompiler::ExportConfig ExportWindow::buildConfig() {
    auto audioConfig = audioConfigWidget->buildConfig();
    auto targetConfig = targetConfigWidget->buildConfig();
    auto codeConfig = codeConfigWidget->buildConfig();

    std::optional<MaximCompiler::ObjectOutputConfig> objectOutputConfig;
    if (outputObjectSection->isChecked() && objectOutputConfigWidget->isConfigValid()) {
        objectOutputConfig = objectOutputConfigWidget->buildConfig();
    }

    std::optional<MaximCompiler::MetaOutputConfig> metaOutputConfig;
    if (outputMetaSection->isChecked() && metaOutputConfigWidget->isConfigValid()) {
        metaOutputConfig = metaOutputConfigWidget->buildConfig();
    }

    MaximCompiler::ExportConfig config(std::move(audioConfig), std::move(targetConfig), std::move(codeConfig),
                                       std::move(objectOutputConfig), std::move(metaOutputConfig));
    return config;
}
