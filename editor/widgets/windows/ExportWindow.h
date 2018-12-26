#pragma once

#include <QtWidgets/QDialog>

namespace AxiomModel {
    class Project;
}

namespace AxiomGui {

    class ExportWindow : public QDialog {
        Q_OBJECT

    public:
        explicit ExportWindow(const AxiomModel::Project &project);
    };
}
