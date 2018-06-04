#include "MainWindow.h"

#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtCore/QTimer>

#include "editor/resources/resource.h"
#include "../surface/NodeSurfacePanel.h"
#include "../modulebrowser/ModuleBrowserPanel.h"
#include "../history/HistoryPanel.h"
#include "AboutWindow.h"
#include "editor/AxiomApplication.h"
#include "editor/model/objects/RootSurface.h"
#include "editor/model/PoolOperators.h"
#include "editor/model/LibraryEntry.h"
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
    connect(GlobalActions::fileImportLibrary, &QAction::triggered,
            this, &MainWindow::importLibrary);
    connect(GlobalActions::fileExportLibrary, &QAction::triggered,
            this, &MainWindow::exportLibrary);

    setProject(std::move(project));
    importLibraryFrom(":/default.axl");
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
            newDockPtr->setFocus(Qt::OtherFocusReason);
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
                                                     tr("Axiom Project Files (*.axp);;All Files (*.*)"));
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

void MainWindow::importLibrary() {
    auto selectedFile = QFileDialog::getOpenFileName(this, "Import Library", QString(), tr("Axiom Library Files (*.axl);;All Files (*.*)"));
    if (selectedFile.isNull()) return;
    importLibraryFrom(selectedFile);
}

void MainWindow::exportLibrary() {
    auto selectedFile = QFileDialog::getSaveFileName(this, "Export Library", QString(), tr("Axiom Library Files (*.axl);;All Files (*.*)"));
    if (selectedFile.isNull()) return;

    QFile file(selectedFile);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox(QMessageBox::Critical, "Failed to export library", "The file you selected couldn't be opened.", QMessageBox::Ok).exec();
        return;
    }

    QDataStream stream(&file);
    AxiomModel::Project::writeHeader(stream, AxiomModel::Project::librarySchemaMagic);
    _project->library().serialize(stream);
    file.close();
}

void MainWindow::importLibraryFrom(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox(QMessageBox::Critical, "Failed to import library", "The file you selected couldn't be opened.", QMessageBox::Ok).exec();
        return;
    }

    QDataStream stream(&file);
    uint32_t readVersion = 0;
    if (!AxiomModel::Project::readHeader(stream, AxiomModel::Project::librarySchemaMagic, &readVersion)) {
        if (readVersion) {
            QMessageBox(QMessageBox::Critical, "Failed to load library",
                        "The file you selected was created with an incompatible version of Axiom.\n\n"
                        "Expected version: between " + QString::number(AxiomModel::Project::minSchemaVersion) +
                        " and " + QString::number(AxiomModel::Project::schemaVersion) + ", actual version: " +
                        QString::number(readVersion) + ".", QMessageBox::Ok).exec();
        } else {
            QMessageBox(QMessageBox::Critical, "Failed to load library",
                        "The file you selected is an invalid library file (bad magic header).\n"
                        "Maybe it's corrupt?", QMessageBox::Ok).exec();
        }
        return;
    }

    AxiomModel::Library mergeLibrary(_project.get(), stream);
    file.close();
    _project->library().import(&mergeLibrary, [](AxiomModel::LibraryEntry *oldEntry, AxiomModel::LibraryEntry *newEntry) {
        auto currentNewer = oldEntry->modificationDateTime() > newEntry->modificationDateTime();

        QMessageBox msgBox(QMessageBox::Warning, "Module import conflict",
                           tr("Heads up! One of the modules in the imported library is conflicting with one you already had.\n\n"
                              "Current module (") + (currentNewer ? "newer" : "older") + ")\n"
                                                                                         "Name: " + oldEntry->name() + "\n"
                                                                                                                       "Last edit: " + oldEntry->modificationDateTime().toLocalTime().toString() + "\n\n"
                                                                                                                                                                                                   "New module (" + (currentNewer ? "older" : "newer") + ")\n"
                                                                                                                                                                                                                                                         "Name: " + newEntry->name() + "\n"
                                                                                                                                                                                                                                                                                       "Last edit: " + newEntry->modificationDateTime().toLocalTime().toString() + "\n\n"
                                                                                                                                                                                                                                                                                                                                                                   "Would you like to keep the current module, imported one, or both?");
        auto currentBtn = msgBox.addButton("Current", QMessageBox::ActionRole);
        auto importedBtn = msgBox.addButton("Imported", QMessageBox::ActionRole);
        msgBox.addButton("Both", QMessageBox::ActionRole);
        msgBox.setDefaultButton(importedBtn);
        msgBox.exec();

        if (msgBox.clickedButton() == currentBtn) return AxiomModel::Library::ConflictResolution::KEEP_OLD;
        else if (msgBox.clickedButton() == importedBtn) return AxiomModel::Library::ConflictResolution::KEEP_NEW;
        else return AxiomModel::Library::ConflictResolution::KEEP_BOTH;
    });
}
