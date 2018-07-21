#pragma once

#include <QtWidgets/QMainWindow>
#include <memory>
#include <unordered_map>

#include "editor/backend/AudioBackend.h"
#include "editor/compiler/interface/Runtime.h"
#include "editor/model/Project.h"

namespace AxiomModel {
    class Project;

    class NodeSurface;
}

namespace AxiomGui {

    class NodeSurfacePanel;

    class HistoryPanel;

    class ModuleBrowserPanel;

    class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        MainWindow(AxiomBackend::AudioBackend *backend);

        ~MainWindow() override;

        MaximCompiler::Runtime *runtime() { return &_runtime; }

        AxiomModel::Project *project() const { return _project.get(); }

        void setProject(std::unique_ptr<AxiomModel::Project> project);

    public slots:

        void showSurface(NodeSurfacePanel *fromPanel, AxiomModel::NodeSurface *schematic, bool split);

        void showAbout();

        void newProject();

    private slots:

        void removeSurface(AxiomModel::NodeSurface *surface);

        void openProject();

        void saveProject();

        void exportProject();

        void importLibrary();

        void exportLibrary();

        void importLibraryFrom(const QString &path);

    private:
        AxiomBackend::AudioBackend *_backend;
        MaximCompiler::Runtime _runtime;
        std::unique_ptr<AxiomModel::Project> _project;
        std::unordered_map<AxiomModel::NodeSurface *, std::unique_ptr<NodeSurfacePanel>> _openPanels;
        std::unique_ptr<HistoryPanel> _historyPanel;
        std::unique_ptr<ModuleBrowserPanel> _modulePanel;
    };
}
