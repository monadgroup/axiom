#include "MainWindow.h"

#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QMenuBar>

#include "src/resources/resource.h"
#include "../modulebrowser/ModuleBrowserPanel.h"
#include "../nodeview/NodeViewPanel.h"

using namespace AxiomGui;

MainWindow::MainWindow() {
    setCentralWidget(nullptr);
    setWindowTitle(tr(VER_PRODUCTNAME_STR));

    resize(1440, 810);

    setUnifiedTitleAndToolBarOnMac(true);
    setDockNestingEnabled(true);
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    // create menu bars
    auto fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(new QAction(tr("&Import Library...")));
    fileMenu->addAction(new QAction(tr("&Export Library...")));
    fileMenu->addSeparator();
    fileMenu->addAction(new QAction(tr("Close Project")));
    fileMenu->addAction(new QAction(tr("Save")));
    fileMenu->addAction(new QAction(tr("Save As...")));
    fileMenu->addSeparator();
    fileMenu->addAction(new QAction(tr("Export...")));
    fileMenu->addSeparator();
    fileMenu->addAction(new QAction(tr("Exit")));

    auto editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(new QAction(tr("Undo")));
    editMenu->addAction(new QAction(tr("Redo")));
    editMenu->addSeparator();
    editMenu->addAction(new QAction(tr("Cut")));
    editMenu->addAction(new QAction(tr("Copy")));
    editMenu->addAction(new QAction(tr("Paste")));
    editMenu->addAction(new QAction(tr("Delete")));
    editMenu->addSeparator();
    editMenu->addAction(new QAction(tr("Select All")));
    editMenu->addSeparator();
    editMenu->addAction(new QAction(tr("Preferences...")));

    auto helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(new QAction(tr("About")));

    // docking for testing
    auto *canvasDock = new NodeViewPanel();
    canvasDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::TopDockWidgetArea, canvasDock);

    auto *moduleBrowser = new ModuleBrowserPanel(this);
    moduleBrowser->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::BottomDockWidgetArea, moduleBrowser);
}
