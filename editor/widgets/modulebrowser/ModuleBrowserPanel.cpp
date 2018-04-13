#include "ModuleBrowserPanel.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QTabBar>
#include <QtWidgets/QLineEdit>

#include "editor/util.h"
#include "ModulePreviewList.h"

using namespace AxiomGui;

ModuleBrowserPanel::ModuleBrowserPanel(AxiomModel::Library *library, QWidget *parent) : DockPanel("Modules", parent) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/ModuleBrowserPanel.qss"));

    auto mainLayout = new QGridLayout(this);
    auto mainWidget = new QWidget(this);
    mainWidget->setObjectName("mainWidget");

    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->setColumnStretch(0, 10);
    mainLayout->setColumnStretch(1, 3);
    mainLayout->setColumnMinimumWidth(1, 200);
    mainLayout->setRowStretch(1, 1);

    auto filterTabs = new QTabBar(this);
    mainLayout->addWidget(filterTabs, 0, 0, Qt::AlignLeft);

    filterTabs->addTab(tr("All"));
    filterTabs->addTab(tr("+"));

    auto searchBox = new QLineEdit(this);
    searchBox->setObjectName("searchBox");
    searchBox->setPlaceholderText("Search modules...");
    mainLayout->addWidget(searchBox, 0, 1);

    auto previewList = new ModulePreviewList(library, this);
    mainLayout->addWidget(previewList, 1, 0, 1, 2);

    mainWidget->setLayout(mainLayout);
    setWidget(mainWidget);
}
