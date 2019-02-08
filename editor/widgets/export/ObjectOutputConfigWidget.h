#pragma once

#include <QtWidgets/QWidget>

#include "editor/compiler/interface/Exporter.h"

namespace AxiomGui {

    class FileBrowserWidget;

    class ObjectOutputConfigWidget : public QWidget {
        Q_OBJECT

    public:
        ObjectOutputConfigWidget();

        bool isConfigValid();

        MaximCompiler::ObjectOutputConfig buildConfig();

    private:
        FileBrowserWidget *outputBrowser;
    };
}
