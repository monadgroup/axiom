#include "AxiomApplication.h"

#include <memory>

#include "util.h"
#include "src/model/CustomNode.h"

AxiomApplication *AxiomApplication::main = nullptr;
AxiomModel::Project *AxiomApplication::project = new AxiomModel::Project();

AxiomApplication::AxiomApplication(int argc, char **argv) : QApplication(argc, argv) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/MainStyles.qss"));

    // make a debug project
    auto node = std::make_unique<AxiomModel::CustomNode>(&project->root);
    node->setPos(QPoint(2, 2));
    node->setSize(QSize(1, 1));
    project->root.addNode(std::move(node));
}
