#pragma once

#include <QtWidgets/QWidget>

#include "editor/compiler/interface/Exporter.h"

namespace AxiomModel {
    class Project;
}

namespace AxiomBackend {
    class AudioConfiguration;
}

namespace AxiomGui {

    class PortalDefinitionEditorWidget;
    class FileBrowserWidget;

    class MetaOutputConfigWidget : public QWidget {
        Q_OBJECT

    public:
        MetaOutputConfigWidget(const AxiomModel::Project &project,
                               const AxiomBackend::AudioConfiguration &configuration);

        bool isConfigValid();

        MaximCompiler::MetaOutputConfig buildConfig();

    public slots:

        void setInstrumentPrefix(const QString &oldSafePrefix, const QString &newSafePrefix);

    private:
        PortalDefinitionEditorWidget *portalEditor;
        FileBrowserWidget *outputBrowser;
    };
}
