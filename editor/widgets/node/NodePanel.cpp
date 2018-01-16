#include "NodePanel.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QCheckBox>

#include "editor/model/node/Node.h"
#include "editor/util.h"

using namespace AxiomGui;
using namespace AxiomModel;

NodePanel::NodePanel(Node *node) : node(node) {
    setStyleSheet(AxiomUtil::loadStylesheet(":/NodePanel.qss"));

    auto wrapperLayout = new QGridLayout();
    wrapperLayout->setMargin(0);
    wrapperWidget = new QWidget();
    wrapperWidget->setObjectName("container");
    wrapperLayout->addWidget(wrapperWidget, 0, 0);
    setLayout(wrapperLayout);

    auto layout = new QGridLayout();
    layout->setMargin(0);
    wrapperWidget->setLayout(layout);

    layout->setColumnStretch(0, 1);

    auto openToggle = new QCheckBox("Open");
    layout->addWidget(openToggle, 0, 2);

    connect(node, &Node::selectedChanged,
            this, &NodePanel::setVisible);

    connect(openToggle, &QCheckBox::stateChanged,
            this, &NodePanel::openToggleChanged);

    setVisible(node->isSelected());
}

void NodePanel::openToggleChanged(int state) {

}
