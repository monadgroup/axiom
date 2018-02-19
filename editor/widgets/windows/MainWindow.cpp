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

    // docking for testing
    auto canvasDock = new SchematicPanel(this, &AxiomApplication::project->root);
    canvasDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::TopDockWidgetArea, canvasDock);

    auto moduleBrowser = new ModuleBrowserPanel();
    moduleBrowser->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::BottomDockWidgetArea, moduleBrowser);
}

void MainWindow::showSchematic(SchematicPanel *fromPanel, AxiomModel::Schematic *schematic, bool split) {
    auto canvasDock = new SchematicPanel(this, schematic);
    canvasDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    if (split) {
        splitDockWidget(fromPanel, canvasDock, Qt::Horizontal);
    } else {
        tabifyDockWidget(fromPanel, canvasDock);
        canvasDock->setFocus();
    }
}

void MainWindow::showAbout() {
    AboutWindow().exec();
}
