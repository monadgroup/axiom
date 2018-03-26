#include "MainWindow.h"

#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QMenuBar>

#include "editor/resources/resource.h"
#include "../modulebrowser/ModuleBrowserPanel.h"
#include "editor/widgets/schematic/SchematicPanel.h"
#include "AboutWindow.h"
#include "editor/AxiomApplication.h"
#include "editor/model/Project.h"

using namespace AxiomGui;

MainWindow::MainWindow() {
    setCentralWidget(nullptr);
    setWindowTitle(tr(VER_PRODUCTNAME_STR));
    setWindowIcon(QIcon(":/application.ico"));

    resize(1440, 810);

    setUnifiedTitleAndToolBarOnMac(true);
    setDockNestingEnabled(true);
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    // create menu bars
    auto fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(new QAction(tr("&Import Library...")));
    fileMenu->addAction(new QAction(tr("E&xport Library...")));
    fileMenu->addSeparator();
    fileMenu->addAction(new QAction(tr("&Open...")));
    fileMenu->addAction(new QAction(tr("&Save As...")));
    fileMenu->addSeparator();
    fileMenu->addAction(new QAction(tr("&Export...")));
    fileMenu->addSeparator();
    fileMenu->addAction(new QAction(tr("Ex&it")));

    auto editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(new QAction(tr("&Undo")));
    editMenu->addAction(new QAction(tr("&Redo")));
    editMenu->addSeparator();
    editMenu->addAction(new QAction(tr("C&ut")));
    editMenu->addAction(new QAction(tr("&Copy")));
    editMenu->addAction(new QAction(tr("&Paste")));
    editMenu->addAction(new QAction(tr("&Delete")));
    editMenu->addSeparator();
    editMenu->addAction(new QAction(tr("&Select All")));
    editMenu->addSeparator();
    editMenu->addAction(new QAction(tr("Pr&eferences...")));

    auto helpMenu = menuBar()->addMenu(tr("&Help"));
    auto aboutAction = new QAction(tr("&About"));
    connect(aboutAction, &QAction::triggered,
            this, &MainWindow::showAbout);
    helpMenu->addAction(aboutAction);
}

void MainWindow::closeProject() {
    // todo
}

void MainWindow::loadProject(AxiomModel::Project *project) {
    closeProject();

    auto moduleBrowser = new ModuleBrowserPanel();
    moduleBrowser->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::BottomDockWidgetArea, moduleBrowser);

    showSchematic(nullptr, &project->root, false);
}

void MainWindow::showSchematic(SchematicPanel *fromPanel, AxiomModel::Schematic *schematic, bool split) {
    auto openPanel = _openPanels.find(schematic);
    if (openPanel != _openPanels.end()) {
        openPanel->second->raise();
        return;
    }

    auto newDock = std::make_unique<SchematicPanel>(this, schematic);
    newDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    if (!fromPanel) {
        addDockWidget(Qt::TopDockWidgetArea, newDock.get());
    } else if (split) {
        splitDockWidget(fromPanel, newDock.get(), Qt::Horizontal);
    } else {
        tabifyDockWidget(fromPanel, newDock.get());
        newDock->raise();
    }

    connect(newDock.get(), &SchematicPanel::closed,
            [this, schematic]() { removeSchematic(schematic); });

    _openPanels.emplace(schematic, std::move(newDock));
}

void MainWindow::showAbout() {
    AboutWindow().exec();
}

void MainWindow::removeSchematic(AxiomModel::Schematic *schematic) {
    _openPanels.erase(schematic);
}
