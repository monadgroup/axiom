#pragma once

#include <QtWidgets/QWidget>

namespace AxiomModel {
    class Project;
}

namespace AxiomBackend {
    class AudioConfiguration;
}

namespace AxiomGui {

    class FileBrowserWidget;

    class MetaOutputConfigWidget : public QWidget {
        Q_OBJECT

    public:
        explicit MetaOutputConfigWidget(const AxiomModel::Project &project,
                                        const AxiomBackend::AudioConfiguration &configuration);

    private:
        FileBrowserWidget *outputBrowser;
    };
}
