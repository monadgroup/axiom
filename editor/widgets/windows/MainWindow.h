#pragma once

#include <QtWidgets/QMainWindow>

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
    };

}
