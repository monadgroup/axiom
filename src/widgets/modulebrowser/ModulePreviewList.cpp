#include <src/util.h>
#include "ModulePreviewList.h"

#include "src/layouts/FlowLayout.h"
#include "ModulePreview.h"

using namespace AxiomGui;

ModulePreviewList::ModulePreviewList(QWidget *parent) : QScrollArea(parent) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/ModulePreviewList.qss"));

    auto widget = new QWidget();
    auto layout = new FlowLayout(-1, 10, 10);

    for (auto i = 0; i < 10; i++) {
        layout->addWidget(new ModulePreview());
    }

    widget->setLayout(layout);
    setWidgetResizable(true);
    setWidget(widget);
}
