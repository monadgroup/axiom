#pragma once

#include <QtWidgets/QWidget>

namespace AxiomGui {

    class FileBrowserWidget;

    class ObjectOutputConfigWidget : public QWidget {
        Q_OBJECT

    public:
        ObjectOutputConfigWidget();

    private:
        FileBrowserWidget *outputBrowser;
    };
}
