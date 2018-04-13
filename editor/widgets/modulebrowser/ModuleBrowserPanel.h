#pragma once

#include <map>

#include "../dock/DockPanel.h"

class QTabBar;

namespace AxiomModel {
    class Library;
}

namespace AxiomGui {

    class ModuleBrowserPanel : public DockPanel {
    Q_OBJECT

    public:
        explicit ModuleBrowserPanel(AxiomModel::Library *library, QWidget *parent = nullptr);

    private slots:

        void addTag(const QString &tag);

        void removeTag(const QString &tag);

        void changeTag(int tag);

    private:

        AxiomModel::Library *library;
        QTabBar *filterTabs;
        std::map<QString, int> tabIndexes;
        std::map<int, QString> indexTabs;
    };

}
