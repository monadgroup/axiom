#pragma once

#include <QtWidgets/QMainWindow>
#include <unordered_map>
#include <memory>

#include "../schematic/SchematicPanel.h"

namespace AxiomModel {
    class Project;

    class Schematic;
}

namespace AxiomGui {

    class SchematicPanel;

    class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        MainWindow();

    public slots:

        void closeProject();

        void loadProject(AxiomModel::Project *project);

        void showSchematic(SchematicPanel *fromPanel, AxiomModel::Schematic *schematic, bool split);

        void showAbout();

    private slots:

        void removeSchematic(AxiomModel::Schematic *schematic);

    private:

        std::unordered_map<AxiomModel::Schematic *, std::unique_ptr<SchematicPanel>> _openPanels;
    };

}
