#include "ModulePreview.h"

#include <QtGui/QPainter>

#include "editor/util.h"

using namespace AxiomGui;

ModulePreview::ModulePreview(QWidget *parent) : QPushButton(parent) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/ModulePreview.qss"));

    setText("A Module");
}
