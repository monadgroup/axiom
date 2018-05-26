#include "MainWindow.h"

#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QTimer>

#include "editor/resources/resource.h"
#include "../surface/NodeSurfacePanel.h"
#include "../modulebrowser/ModuleBrowserPanel.h"
#include "../history/HistoryPanel.h"
#include "AboutWindow.h"
#include "editor/AxiomApplication.h"
#include "editor/model/objects/RootSurface.h"
#include "editor/model/PoolOperators.h"
#include "compiler/runtime/Runtime.h"
#include "../GlobalActions.h"

using namespace AxiomGui;

MainWindow::MainWindow(MaximRuntime::Runtime *runtime, std::unique_ptr<AxiomModel::Project> project) : runtime(runtime) {
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

    connect(GlobalActions::helpAbout, &QAction::triggered,
            this, &MainWindow::showAbout);

    auto helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(GlobalActions::helpAbout);

    // connect menu things
    connect(GlobalActions::fileOpen, &QAction::triggered,
            this, &MainWindow::openProject);
    connect(GlobalActions::fileSave, &QAction::triggered,
            this, &MainWindow::saveProject);
    connect(GlobalActions::fileQuit, &QAction::triggered,
            QApplication::quit);

    connect(GlobalActions::editUndo, &QAction::triggered,
            this, [this]() { if (_project) _project->mainRoot().history().undo(); });
    connect(GlobalActions::editRedo, &QAction::triggered,
            this, [this]() { if (_project) _project->mainRoot().history().redo(); });

    setProject(std::move(project));
}

MainWindow::~MainWindow() = default;

void MainWindow::showSurface(NodeSurfacePanel *fromPanel, AxiomModel::NodeSurface *surface, bool split) {
    auto openPanel = _openPanels.find(surface);
    if (openPanel != _openPanels.end()) {
        openPanel->second->raise();
        return;
    }

    auto newDock = std::make_unique<NodeSurfacePanel>(this, surface);
    auto newDockPtr = newDock.get();
    newDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    if (!fromPanel) {
        addDockWidget(Qt::LeftDockWidgetArea, newDockPtr);
    } else if (split) {
        splitDockWidget(fromPanel, newDockPtr, Qt::Horizontal);
    } else {
        tabifyDockWidget(fromPanel, newDockPtr);

        // raise() doesn't seem to work when called synchronously after tabifyDockWidget, so we wait for the next
        // event loop iteration
        QTimer::singleShot(0, newDockPtr, [newDockPtr]() {
            newDockPtr->raise();
        });
    }

    connect(newDockPtr, &NodeSurfacePanel::closed,
            this, [this, surface]() { removeSurface(surface); });
    _openPanels.emplace(surface, std::move(newDock));
}

void MainWindow::showAbout() {
    AboutWindow().exec();
}

void MainWindow::setProject(std::unique_ptr<AxiomModel::Project> project) {
    // cleanup old project state
    if (_historyPanel) {
        _historyPanel->close();
    }
    if (_modulePanel) {
        _modulePanel->close();
    }

    _project = std::move(project);
    _project->attachRuntime(runtime);
    auto &history = _project->mainRoot().history();

    GlobalActions::editUndo->setEnabled(history.canUndo());
    history.canUndoChanged.forward(GlobalActions::editUndo, &QAction::setEnabled);
    history.undoTypeChanged.connect([](AxiomModel::Action::ActionType type) {
        GlobalActions::editUndo->setText("&Undo " + AxiomModel::Action::typeToString(type));
    });

    GlobalActions::editRedo->setEnabled(history.canRedo());
    history.canRedoChanged.forward(GlobalActions::editRedo, &QAction::setEnabled);
    history.redoTypeChanged.connect([](AxiomModel::Action::ActionType type) {
        GlobalActions::editRedo->setText("&Redo " + AxiomModel::Action::typeToString(type));
    });

    // find root surface and show it
    auto defaultSurface = AxiomModel::getFirst(
        AxiomModel::findChildrenWatch(_project->mainRoot().nodeSurfaces(), QUuid()));
    assert(defaultSurface.value());
    showSurface(nullptr, *defaultSurface.value(), false);

    _historyPanel = std::make_unique<HistoryPanel>(&_project->mainRoot().history(), this);
    addDockWidget(Qt::RightDockWidgetArea, _historyPanel.get());

    _modulePanel = std::make_unique<ModuleBrowserPanel>(this, &_project->library(), this);
    addDockWidget(Qt::BottomDockWidgetArea, _modulePanel.get());
}

void MainWindow::removeSurface(AxiomModel::NodeSurface *surface) {
    _openPanels.erase(surface);
}

void MainWindow::saveProject() {
    auto selectedFile = QFileDialog::getSaveFileName(this, "Save Project", QString(),
                                                     tr("Axiom Project Files (*.axp);;All Files(*.*)"));
    if (selectedFile.isNull()) return;

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

void MainWindow::openProject() {
    auto selectedFile = QFileDialog::getOpenFileName(this, "Open Project", QString(),
                                                     tr("Axiom Project Files (*.axp);;All Files (*.*)"));
    if (selectedFile.isNull()) return;

    QFile file(selectedFile);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox(QMessageBox::Critical, "Failed to open project", "The file you selected couldn't be opened.",
                    QMessageBox::Ok).exec();
        return;
    }

    QDataStream stream(&file);
    uint32_t readVersion = 0;
    auto newProject = AxiomModel::Project::deserialize(stream, &readVersion);
    file.close();

    if (!newProject) {
        if (readVersion) {
            QMessageBox(QMessageBox::Critical, "Failed to load project",
                        "The file you selected was created with an incompatible version of Axiom.\n\n"
                        "Expected version: between " + QString::number(AxiomModel::Project::minSchemaVersion) +
                        " and " + QString::number(AxiomModel::Project::schemaVersion) + ", actual version: " +
                        QString::number(readVersion) + ".", QMessageBox::Ok).exec();
        } else {
            QMessageBox(QMessageBox::Critical, "Failed to load project",
                        "The file you selected is an invalid project file (bad magic header).\n"
                        "Maybe it's corrupt?", QMessageBox::Ok).exec();
        }
    } else {
        setProject(std::move(newProject));
    }
}
