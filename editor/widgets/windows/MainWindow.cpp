#include "MainWindow.h"

#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QShortcut>

#include "editor/resources/resource.h"
#include "../modulebrowser/ModuleBrowserPanel.h"
#include "editor/widgets/schematic/SchematicPanel.h"
#include "AboutWindow.h"
#include "editor/AxiomApplication.h"
#include "editor/model/Project.h"

using namespace AxiomGui;

MainWindow::MainWindow(AxiomModel::Project *project) {
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
    auto undoAction = editMenu->addAction(tr("&Undo"));
    auto redoAction = editMenu->addAction(tr("&Redo"));

    undoAction->setShortcut(QKeySequence(tr("Ctrl+Z", "Edit|Undo")));
    redoAction->setShortcut(QKeySequence(tr("Ctrl+Y", "Edit|Redo")));
    undoAction->setEnabled(project->history.canUndo());
    redoAction->setEnabled(project->history.canRedo());
    connect(&project->history, &AxiomModel::HistoryList::canUndoChanged,
            undoAction, &QAction::setEnabled);
    connect(&project->history, &AxiomModel::HistoryList::canRedoChanged,
            redoAction, &QAction::setEnabled);
    connect(undoAction, &QAction::triggered,
            &project->history, &AxiomModel::HistoryList::undo);
    connect(redoAction, &QAction::triggered,
            &project->history, &AxiomModel::HistoryList::redo);

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
    auto aboutAction = helpMenu->addAction(tr("&About"));
    connect(aboutAction, &QAction::triggered,
            this, &MainWindow::showAbout);

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
    auto newDockPtr = newDock.get();
    newDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    if (!fromPanel) {
        addDockWidget(Qt::TopDockWidgetArea, newDockPtr);
    } else if (split) {
        splitDockWidget(fromPanel, newDockPtr, Qt::Horizontal);
    } else {
        tabifyDockWidget(fromPanel, newDockPtr);

        // raise() doesn't seem to work when called synchronously after tabifyDockWidget, so we wait for the next
        // event loop iteration
        // fixme: if the dock is removed before this timer finishes, we'll segfault
        QTimer::singleShot(0, this, [newDockPtr]() {
            newDockPtr->raise();
        });
    }

    connect(newDockPtr, &SchematicPanel::closed,
            [this, schematic]() { removeSchematic(schematic); });

    _openPanels.emplace(schematic, std::move(newDock));
}

void MainWindow::showAbout() {
    AboutWindow().exec();
}

void MainWindow::removeSchematic(AxiomModel::Schematic *schematic) {
    _openPanels.erase(schematic);
}
