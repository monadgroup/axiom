#pragma once

#include "../dock/DockPanel.h"

namespace AxiomModel {
    class Library;
}

namespace AxiomGui {

    class ModuleBrowserPanel : public DockPanel {
    Q_OBJECT

    public:
        explicit ModuleBrowserPanel(AxiomModel::Library *library, QWidget *parent = nullptr);
    };

}
