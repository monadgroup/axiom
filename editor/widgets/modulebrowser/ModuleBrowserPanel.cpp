#include "ModuleBrowserPanel.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTabBar>

#include "ModulePreviewList.h"
#include "editor/model/Library.h"
#include "editor/util.h"

using namespace AxiomGui;

ModuleBrowserPanel::ModuleBrowserPanel(MainWindow *window, AxiomModel::Library *library, QWidget *parent)
    : ads::CDockWidget("Modules", parent), library(library) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/styles/ModuleBrowserPanel.qss"));

    auto mainWidget = new QWidget(this);
    mainWidget->setObjectName("mainWidget");
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setMargin(0);
    mainWidget->setLayout(mainLayout);

    auto filterBarWidget = new QWidget(this);
    filterBarWidget->setObjectName("filterBar");
    auto filterBarLayout = new QHBoxLayout(this);
    filterBarLayout->setContentsMargins(0, 0, 0, 0);
    filterBarLayout->setMargin(0);
    filterBarWidget->setLayout(filterBarLayout);

    mainLayout->addWidget(filterBarWidget);

    filterTabs = new QTabBar(this);
    filterTabs->addTab(tr("All"));
    filterBarLayout->addWidget(filterTabs, 10, Qt::AlignLeft | Qt::AlignBottom);

    searchBox = new QLineEdit(this);
    searchBox->setObjectName("searchBox");
    searchBox->setPlaceholderText("Search modules...");
    searchBox->setText(library->activeSearch());
    filterBarLayout->addWidget(searchBox, 3);

    auto previewList = new ModulePreviewList(window, library, this);
    mainLayout->addWidget(previewList, 1);

    setWidget(mainWidget);

    auto tags = library->tags();
    for (const auto &tag : tags) {
        addTag(tag);
    }

    library->tagAdded.connect(this, &ModuleBrowserPanel::addTag);
    library->tagRemoved.connect(this, &ModuleBrowserPanel::removeTag);
    connect(filterTabs, &QTabBar::currentChanged, this, &ModuleBrowserPanel::changeTag);
}

void ModuleBrowserPanel::addTag(const QString &tag) {
    filterTabs->addTab(tag);
    tabValues.push_back(tag);
}

void ModuleBrowserPanel::removeTag(const QString &tag) {
    auto index = std::find(tabValues.begin(), tabValues.end(), tag);
    assert(index != tabValues.end());
    tabValues.erase(index);
    auto tabIndex = index - tabValues.begin() + 1;
    if (filterTabs->currentIndex() == tabIndex) {
        filterTabs->setCurrentIndex(0);
    }
    filterTabs->removeTab(tabIndex);
}

void ModuleBrowserPanel::changeTag(int tag) {
    searchBox->setText("");
    searchBox->clearFocus();
    if (tag == 0)
        library->setActiveTag("");
    else {
        assert(tag <= (int) tabValues.size());
        library->setActiveTag(tabValues[tag - 1]);
    }
}

void ModuleBrowserPanel::changeSearch(const QString &newSearch) {
    library->setActiveSearch(newSearch);
}
