#pragma once

#include <QtWidgets/QMainWindow>
#include <unordered_map>
#include <memory>

#include "editor/model/Project.h"
#include "editor/widgets/schematic/NodeSurfacePanel.h"

namespace AxiomModel {
    class Project;

    class NodeSurface;
}

namespace AxiomGui {

    class NodeSurfacePanel;

    class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        MainWindow(AxiomModel::Project project);

    public slots:

        //void showSchematic(SchematicPanel *fromPanel, AxiomModel::NodeSurface *schematic, bool split);

        void showAbout();

    private slots:

        /*void removeSchematic(AxiomModel::Schematic *schematic);

        void openProject();

        void saveProject();*/

    private:

        AxiomModel::Project _project;
        std::unordered_map<AxiomModel::NodeSurface *, std::unique_ptr<NodeSurfacePanel>> _openPanels;
    };

}
