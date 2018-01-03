#pragma once

#include <QtWidgets/QDockWidget>

namespace AxiomGui {

    class DockPanel : public QDockWidget {
    Q_OBJECT

    public:
        DockPanel(const QString &title);
    };

}
