#include "NodePanel.h"

#include <QtWidgets/QGridLayout>
#include <QtCore/QVariant>
#include <QtWidgets/QStyle>
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

    auto lockToggle = new QCheckBox("Lock");
    layout->addWidget(lockToggle, 0, 1);

    auto openToggle = new QCheckBox("Open");
    layout->addWidget(openToggle, 0, 2);

    connect(node, &Node::selectedChanged,
            this, &NodePanel::setVisible);

    connect(&node->surface, &NodeSurface::lockedChanged,
            [this, lockToggle](bool locked) { lockToggle->setChecked(locked); });
    connect(lockToggle, &QCheckBox::stateChanged,
            this, &NodePanel::lockToggleChanged);

    connect(openToggle, &QCheckBox::stateChanged,
            this, &NodePanel::openToggleChanged);

    lockToggle->setChecked(node->surface.locked());
}

void NodePanel::lockToggleChanged(int state) {
    node->surface.setLocked(state == Qt::Checked);
}

void NodePanel::openToggleChanged(int state) {

}
