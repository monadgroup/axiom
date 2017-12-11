#pragma once
#include <QtWidgets/QDockWidget>

namespace AxiomGui {

    class ModuleBrowserPanel : public QDockWidget {
        Q_OBJECT

    public:
        explicit ModuleBrowserPanel(QWidget *parent = nullptr);
    };

}
