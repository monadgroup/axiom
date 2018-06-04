#pragma once

#include <QtWidgets/QMainWindow>
#include <unordered_map>
#include <memory>

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
        MainWindow(MaximRuntime::Runtime *runtime, std::unique_ptr<AxiomModel::Project> project);

        ~MainWindow() override;

        AxiomModel::Project *project() const { return _project.get(); }

        void setProject(std::unique_ptr<AxiomModel::Project> project);

    public slots:

        void showSurface(NodeSurfacePanel *fromPanel, AxiomModel::NodeSurface *schematic, bool split);

        void showAbout();

    private slots:

        void removeSurface(AxiomModel::NodeSurface *surface);

        void openProject();

        void saveProject();

        void importLibrary();

        void exportLibrary();

    private:

        MaximRuntime::Runtime *runtime;
        std::unique_ptr<AxiomModel::Project> _project;
        std::unordered_map<AxiomModel::NodeSurface *, std::unique_ptr<NodeSurfacePanel>> _openPanels;
        std::unique_ptr<HistoryPanel> _historyPanel;
        std::unique_ptr<ModuleBrowserPanel> _modulePanel;

        void importLibraryFrom(const QString &path);
    };

}
