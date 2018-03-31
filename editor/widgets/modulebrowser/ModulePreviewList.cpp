#include "ModulePreviewList.h"

#include "../layouts/FlowLayout.h"
#include "editor/util.h"
#include "ModulePreview.h"

using namespace AxiomGui;

ModulePreviewList::ModulePreviewList(QWidget *parent) : QScrollArea(parent) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/ModulePreviewList.qss"));

    auto widget = new QWidget(this);
    auto layout = new FlowLayout(this, -1, 10, 10);

    for (auto i = 0; i < 10; i++) {
        layout->addWidget(new ModulePreview(this));
    }

    widget->setLayout(layout);
    setWidgetResizable(true);
    setWidget(widget);
}
