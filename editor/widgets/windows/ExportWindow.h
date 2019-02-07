#pragma once

#include <QtWidgets/QDialog>

#include "editor/compiler/interface/Exporter.h"

class QGroupBox;

namespace AxiomModel {
    class Project;
}

namespace AxiomGui {
    class AudioConfigWidget;
    class TargetConfigWidget;
    class CodeConfigWidget;
    class ObjectOutputConfigWidget;
    class MetaOutputConfigWidget;

    class ExportWindow : public QDialog {
        Q_OBJECT

    public:
        explicit ExportWindow(const AxiomModel::Project &project);

    private slots:
        void doExport();

    private:
        const AxiomModel::Project &project;

        AudioConfigWidget *audioConfigWidget;
        TargetConfigWidget *targetConfigWidget;
        CodeConfigWidget *codeConfigWidget;
        QGroupBox *outputObjectSection;
        ObjectOutputConfigWidget *objectOutputConfigWidget;
        QGroupBox *outputMetaSection;
        MetaOutputConfigWidget *metaOutputConfigWidget;

        MaximCompiler::ExportConfig buildConfig();
    };
}
