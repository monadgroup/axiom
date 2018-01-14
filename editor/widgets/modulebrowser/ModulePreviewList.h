#pragma once

#include <QtWidgets/QScrollArea>

namespace AxiomGui {

    class ModulePreviewList : public QScrollArea {
    Q_OBJECT

    public:
        explicit ModulePreviewList(QWidget *parent = nullptr);
    };

}
