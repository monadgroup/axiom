#include "MainWindow.h"

#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

#include "editor/resources/resource.h"
#include "../modulebrowser/ModuleBrowserPanel.h"
#include "../history/HistoryPanel.h"
#include "AboutWindow.h"
#include "editor/AxiomApplication.h"
#include "editor/model/Project.h"
#include "../GlobalActions.h"

using namespace AxiomGui;

MainWindow::MainWindow(AxiomModel::Project *project) : _project(project) {
    setCentralWidget(nullptr);
    setWindowTitle(tr(VER_PRODUCTNAME_STR));
    setWindowIcon(QIcon(":/application.ico"));

    resize(1440, 810);

    setUnifiedTitleAndToolBarOnMac(true);
    setDockNestingEnabled(true);
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    // build
    auto fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(GlobalActions::fileImportLibrary);
    fileMenu->addAction(GlobalActions::fileExportLibrary);
    fileMenu->addSeparator();

    fileMenu->addAction(GlobalActions::fileOpen);
    fileMenu->addAction(GlobalActions::fileSave);
    fileMenu->addSeparator();

    fileMenu->addAction(GlobalActions::fileExport);
    fileMenu->addSeparator();

    fileMenu->addAction(GlobalActions::fileQuit);

    auto editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(GlobalActions::editUndo);
    editMenu->addAction(GlobalActions::editRedo);
    editMenu->addSeparator();

    editMenu->addAction(GlobalActions::editCut);
    editMenu->addAction(GlobalActions::editCopy);
    editMenu->addAction(GlobalActions::editPaste);
    editMenu->addAction(GlobalActions::editDelete);
    editMenu->addSeparator();

    editMenu->addAction(GlobalActions::editSelectAll);
    editMenu->addSeparator();

    editMenu->addAction(GlobalActions::editPreferences);

    auto helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(GlobalActions::helpAbout);

    // connect menu things
    connect(GlobalActions::fileOpen, &QAction::triggered,
            this, &MainWindow::openProject);
    connect(GlobalActions::fileSave, &QAction::triggered,
            this, &MainWindow::saveProject);
    connect(GlobalActions::fileQuit, &QAction::triggered,
            QApplication::quit);

    GlobalActions::editUndo->setEnabled(project->history.canUndo());
    connect(&project->history, &AxiomModel::HistoryList::canUndoChanged,
            GlobalActions::editUndo, &QAction::setEnabled);
    connect(&project->history, &AxiomModel::HistoryList::undoTypeChanged,
            [](AxiomModel::HistoryList::ActionType type) {
                GlobalActions::editUndo->setText("&Undo " + AxiomModel::HistoryList::typeToString(type));
            });
    connect(GlobalActions::editUndo, &QAction::triggered,
            &project->history, &AxiomModel::HistoryList::undo);

    GlobalActions::editRedo->setEnabled(project->history.canRedo());
    connect(&project->history, &AxiomModel::HistoryList::canRedoChanged,
            GlobalActions::editRedo, &QAction::setEnabled);
    connect(&project->history, &AxiomModel::HistoryList::undoTypeChanged,
            [](AxiomModel::HistoryList::ActionType type) {
                GlobalActions::editRedo->setText("&Redo " + AxiomModel::HistoryList::typeToString(type));
            });
    connect(GlobalActions::editRedo, &QAction::triggered,
            &project->history, &AxiomModel::HistoryList::redo);

    connect(GlobalActions::helpAbout, &QAction::triggered,
            this, &MainWindow::showAbout);

    auto historyList = new HistoryPanel(&project->history, this);
    historyList->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, historyList);

    auto moduleBrowser = new ModuleBrowserPanel(this, &project->library, this);
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
            this, [this, schematic]() { removeSchematic(schematic); });

    _openPanels.emplace(schematic, std::move(newDock));
}

void MainWindow::showAbout() {
    AboutWindow().exec();
}

void MainWindow::removeSchematic(AxiomModel::Schematic *schematic) {
    _openPanels.erase(schematic);
}

void MainWindow::openProject() {
    auto selectedFile = QFileDialog::getOpenFileName(this, "Open Project", QString(),
                                                     tr("Axiom Project Files (*.axp);;All Files (*.*)"));
    if (selectedFile == QString()) return;

    QFile file(selectedFile);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox(QMessageBox::Critical, "Failed to open project", "The file you selected couldn't be opened.",
                    QMessageBox::Ok).exec();
        return;
    }

    QDataStream stream(&file);
    try {
        _project->load(stream);
    } catch (AxiomModel::DeserializeInvalidFileException) {
        QMessageBox(QMessageBox::Critical, "Failed to load project",
                    "The file you selected is an invalid project file (bad magic header).\n"
                    "Maybe it's corrupt?", QMessageBox::Ok).exec();
    } catch (AxiomModel::DeserializeInvalidSchemaException) {
        QMessageBox(QMessageBox::Critical, "Failed to load project",
                    "The file you selected is saved with an outdated project format.\n\n"
                    "Unfortunately Axiom currently doesn't support loading old project formats.\n"
                    "If you really want this feature, maybe make a pull request (https://github.com/monadgroup/axiom/pulls)"
                    " or report an issue (https://github.com/monadgroup/axiom/issues).", QMessageBox::Ok).exec();
    }
    file.close();
}

void MainWindow::saveProject() {
    auto selectedFile = QFileDialog::getSaveFileName(this, "Save Project", QString(),
                                                     tr("Axiom Project Files (*.axp);;All Files (*.*)"));
    if (selectedFile == QString()) return;

    QFile file(selectedFile);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox(QMessageBox::Critical, "Failed to save project", "The file you selected couldn't be opened.",
                    QMessageBox::Ok).exec();
        return;
    }

    QDataStream stream(&file);
    _project->serialize(stream);
    file.close();
}
