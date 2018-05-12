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

    class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        MainWindow(std::unique_ptr<AxiomModel::Project> project);

        AxiomModel::Project *project() const { return _project.get(); }

        void setProject(std::unique_ptr<AxiomModel::Project> project);

    public slots:

        void showSurface(NodeSurfacePanel *fromPanel, AxiomModel::NodeSurface *schematic, bool split);

        void showAbout();

    private slots:

        void removeSurface(AxiomModel::NodeSurface *surface);

        void openProject();

        void saveProject();

    private:

        std::unique_ptr<AxiomModel::Project> _project;
        std::unordered_map<AxiomModel::NodeSurface *, std::unique_ptr<NodeSurfacePanel>> _openPanels;
    };

}
