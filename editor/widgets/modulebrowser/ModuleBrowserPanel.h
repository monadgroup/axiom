#pragma once

#include <map>

#include "common/Hookable.h"
#include "../dock/DockPanel.h"

class QTabBar;

namespace AxiomModel {
    class Library;
}

namespace AxiomGui {

    class MainWindow;

    class ModuleBrowserPanel : public DockPanel, public AxiomCommon::Hookable {
    Q_OBJECT

    public:
        explicit ModuleBrowserPanel(MainWindow *window, AxiomModel::Library *library, QWidget *parent = nullptr);

    private slots:

        void changeTag(int tag);

    private:

        AxiomModel::Library *library;
        QTabBar *filterTabs;
        std::vector<QString> tabValues;

        void addTag(const QString &tag);

        void removeTag(const QString &tag);
    };

}
