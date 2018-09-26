#pragma once

#include <QtCore/QFileSystemWatcher>
#include <QtCore/QLockFile>
#include <QtCore/QTimer>
#include <QtWidgets/QMainWindow>
#include <memory>
#include <unordered_map>

#include "editor/backend/AudioBackend.h"
#include "editor/compiler/interface/Runtime.h"
#include "editor/model/Project.h"

namespace AxiomModel {
    class Project;

    class Library;

    class NodeSurface;
}

namespace AxiomGui {

    class NodeSurfacePanel;

    class HistoryPanel;

    class ModuleBrowserPanel;

    class MainWindow : public QMainWindow, public AxiomCommon::Hookable {
        Q_OBJECT

    public:
        explicit MainWindow(AxiomBackend::AudioBackend *backend);

        ~MainWindow() override;

        MaximCompiler::Runtime *runtime() { return &_runtime; }

        AxiomModel::Project *project() const { return _project.get(); }

        void setProject(std::unique_ptr<AxiomModel::Project> project);

        static QString globalLibraryLockPath();

        static QString globalLibraryFilePath();

        void lockGlobalLibrary();

        void unlockGlobalLibrary();

        void testLockGlobalLibrary();

    public slots:

        NodeSurfacePanel *showSurface(NodeSurfacePanel *fromPanel, AxiomModel::NodeSurface *schematic, bool split,
                                      bool permanent);

        void showAbout();

        void newProject();

    protected:
        void closeEvent(QCloseEvent *event) override;

        bool event(QEvent *event) override;

    private slots:

        void removeSurface(AxiomModel::NodeSurface *surface);

        void openProject();

        void saveProject();

        void saveAsProject();

        void exportProject();

        void importLibrary();

        void exportLibrary();

        void importLibraryFrom(const QString &path);

    private:
        AxiomBackend::AudioBackend *_backend;
        MaximCompiler::Runtime _runtime;
        std::unique_ptr<AxiomModel::Project> _project;
        std::unique_ptr<AxiomModel::Library> _library;
        std::unordered_map<AxiomModel::NodeSurface *, std::unique_ptr<NodeSurfacePanel>> _openPanels;
        std::unique_ptr<HistoryPanel> _historyPanel;
        std::unique_ptr<ModuleBrowserPanel> _modulePanel;
        QMenu *_viewMenu;
        QLockFile libraryLock;
        bool isLibraryLocked = false;
        QTimer saveDebounceTimer;
        QTimer loadDebounceTimer;
        QFileSystemWatcher globalLibraryWatcher;

        bool didJustSaveLibrary = false;
        bool didJustLoadLibrary = false;

        static std::unique_ptr<AxiomModel::Library> loadGlobalLibrary();
        static std::unique_ptr<AxiomModel::Library> loadDefaultLibrary();

        void saveGlobalLibrary();

        void triggerLibraryChanged();

        void saveProjectTo(const QString &path);

        bool checkCloseProject();

        void updateWindowTitle(const QString &linkedFile, bool isDirty);

    private slots:
        void triggerLibraryChangeDebounce();

        void triggerLibraryReload();

        void triggerLibraryReloadDebounce();
    };
}
