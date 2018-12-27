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
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>

#include "../export/AudioConfigWidget.h"
#include "../export/CodeConfigWidget.h"
#include "../export/MetaOutputConfigWidget.h"
#include "../export/ObjectOutputConfigWidget.h"
#include "../export/TargetConfigWidget.h"
#include "editor/model/Project.h"
#include "editor/util.h"

using namespace AxiomGui;

ExportWindow::ExportWindow(const AxiomModel::Project &project)
    : QDialog(nullptr, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint) {
    setWindowTitle(tr("Export Project"));
    setStyleSheet(AxiomUtil::loadStylesheet(":/styles/UnstyledWindow.qss"));
    setWindowIcon(QIcon(":/application.ico"));

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto mainLayout = new QFormLayout();

    auto audioSection = new QGroupBox("Audio");
    auto audioLayout = new QGridLayout();
    audioLayout->addWidget(new AudioConfigWidget(project), 0, 0);
    audioSection->setLayout(audioLayout);
    mainLayout->addRow(audioSection);

    auto targetSection = new QGroupBox("Target");
    auto targetLayout = new QGridLayout();
    targetLayout->addWidget(new TargetConfigWidget(), 0, 0);
    targetSection->setLayout(targetLayout);
    mainLayout->addRow(targetSection);

    auto codeSection = new QGroupBox("Code");
    auto codeLayout = new QGridLayout();
    auto codeConfig = new CodeConfigWidget();
    codeLayout->addWidget(codeConfig, 0, 0);
    codeSection->setLayout(codeLayout);
    mainLayout->addRow(codeSection);

    auto instrumentOutputSection = new QGroupBox("Output");
    auto instrumentOutputLayout = new QHBoxLayout();
    instrumentOutputSection->setLayout(instrumentOutputLayout);
    mainLayout->addRow(instrumentOutputSection);

    auto outputObjectSection = new QGroupBox("Object file");
    outputObjectSection->setCheckable(true);
    outputObjectSection->setChecked(true);
    auto outputObjectLayout = new QGridLayout();
    outputObjectLayout->addWidget(new ObjectOutputConfigWidget(), 0, 0);
    outputObjectSection->setLayout(outputObjectLayout);
    instrumentOutputLayout->addWidget(outputObjectSection);

    auto outputMetaSection = new QGroupBox("Meta file");
    outputMetaSection->setCheckable(true);
    outputMetaSection->setChecked(true);
    auto outputMetaLayout = new QGridLayout();
    auto audioConfiguration = project.getAudioConfiguration();
    std::sort(audioConfiguration.portals.begin(), audioConfiguration.portals.end());
    auto metaOutputConfig = new MetaOutputConfigWidget(project, audioConfiguration);
    outputMetaLayout->addWidget(metaOutputConfig, 0, 0);
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

    connect(codeConfig, &CodeConfigWidget::instrumentPrefixChanged, metaOutputConfig,
            &MetaOutputConfigWidget::setInstrumentPrefix);
    connect(cancelButton, &QPushButton::clicked, this, &ExportWindow::close);
}
