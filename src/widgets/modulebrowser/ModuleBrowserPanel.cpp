#include "ModuleBrowserPanel.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QTabBar>
#include <QtWidgets/QLineEdit>

#include "src/util.h"
#include "ModulePreviewList.h"

using namespace AxiomGui;

ModuleBrowserPanel::ModuleBrowserPanel(QWidget *parent) : QDockWidget("Modules", parent) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/ModuleBrowserPanel.qss"));

    auto mainLayout = new QGridLayout();
    auto mainWidget = new QWidget();
    mainWidget->setObjectName("mainWidget");

    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->setColumnStretch(0, 10);
    mainLayout->setColumnStretch(1, 3);
    mainLayout->setColumnMinimumWidth(1, 200);
    mainLayout->setRowStretch(1, 1);

    auto filterTabs = new QTabBar();
    mainLayout->addWidget(filterTabs, 0, 0, Qt::AlignLeft);

    filterTabs->addTab(tr("Test 1"));
    filterTabs->addTab(tr("Test 2"));
    filterTabs->addTab(tr("+"));

    auto searchBox = new QLineEdit();
    searchBox->setObjectName("searchBox");
    searchBox->setPlaceholderText("Search modules...");
    mainLayout->addWidget(searchBox, 0, 1);

    auto previewList = new ModulePreviewList();
    mainLayout->addWidget(previewList, 1, 0, 1, 2);

    mainWidget->setLayout(mainLayout);
    setWidget(mainWidget);
    setTitleBarWidget(new QWidget());
}
