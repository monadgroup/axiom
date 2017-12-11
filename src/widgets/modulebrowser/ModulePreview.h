#pragma once
#include <QtWidgets/QPushButton>

namespace AxiomGui {

    class ModulePreview : public QPushButton {
        Q_OBJECT

    public:
        ModulePreview(QWidget *parent = nullptr);
    };

}
