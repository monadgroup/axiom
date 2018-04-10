#include "MainWindow.h"

#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QShortcut>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

#include "editor/resources/resource.h"
#include "../modulebrowser/ModuleBrowserPanel.h"
#include "editor/widgets/schematic/SchematicPanel.h"
#include "AboutWindow.h"
#include "editor/AxiomApplication.h"
#include "editor/model/Project.h"

using namespace AxiomGui;

MainWindow::MainWindow(AxiomModel::Project *project) : _project(project) {
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

    auto openAction = fileMenu->addAction(tr("&Open..."));
    connect(openAction, &QAction::triggered,
            this, &MainWindow::openProject);

    auto saveAction = fileMenu->addAction(tr("&Save As..."));
    connect(saveAction, &QAction::triggered,
            this, &MainWindow::saveProject);

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
    connect(&project->history, &AxiomModel::HistoryList::undoTypeChanged,
            [undoAction](AxiomModel::HistoryList::ActionType type) {
                undoAction->setText("&Undo " + AxiomModel::HistoryList::typeToString(type));
            });
    connect(&project->history, &AxiomModel::HistoryList::redoTypeChanged,
            [redoAction](AxiomModel::HistoryList::ActionType type) {
                redoAction->setText("&Redo " + AxiomModel::HistoryList::typeToString(type));
            });

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

void MainWindow::openProject() {
    auto selectedFile = QFileDialog::getOpenFileName(this, "Open Project", QString(), tr("Axiom Project Files (*.axp);;All Files (*.*)"));
    if (selectedFile == QString()) return;

    QFile file(selectedFile);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox(QMessageBox::Critical, "Failed to open project", "The file you selected couldn't be opened.", QMessageBox::Ok).exec();
        return;
    }

    QDataStream stream(&file);
    try {
        _project->load(stream);
    } catch (AxiomModel::DeserializeInvalidFileException) {
        QMessageBox(QMessageBox::Critical, "Failed to load project", "The file you selected is an invalid project file (bad magic header).\n"
                                                                     "Maybe it's corrupt?", QMessageBox::Ok).exec();
    } catch (AxiomModel::DeserializeInvalidSchemaException) {
        QMessageBox(QMessageBox::Critical, "Failed to load project", "The file you selected is saved with an outdated project format.\n\n"
                                                                     "Unfortunately Axiom currently doesn't support loading old project formats.\n"
                                                                     "If you really want this feature, maybe make a pull request (https://github.com/monadgroup/axiom/pulls)"
                                                                     " or report an issue (https://github.com/monadgroup/axiom/issues).", QMessageBox::Ok).exec();
    }
    file.close();
}

void MainWindow::saveProject() {
    auto selectedFile = QFileDialog::getSaveFileName(this, "Save Project", QString(), tr("Axiom Project Files (*.axp);;All Files (*.*)"));
    if (selectedFile == QString()) return;

    QFile file(selectedFile);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox(QMessageBox::Critical, "Failed to save project", "The file you selected couldn't be opened.", QMessageBox::Ok).exec();
        return;
    }

    QDataStream stream(&file);
    _project->serialize(stream);
    file.close();
}
