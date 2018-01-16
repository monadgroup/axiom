#include "NodePanel.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QCheckBox>
#include <QtCore/QTimer>

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

    auto openToggle = new QCheckBox();
    openToggle->setObjectName("openCheck");
    layout->addWidget(openToggle, 0, 2);

    connect(openToggle, &QCheckBox::stateChanged,
            this, &NodePanel::openToggleChanged);

    connect(node, &Node::selectedChanged,
            this, &NodePanel::updateVisible);
}

void NodePanel::enterEvent(QEvent *event) {
    std::cout << "Enter event" << std::endl;
    setSelfHover(true);
}

void NodePanel::leaveEvent(QEvent *event) {
    std::cout << "Leave event" << std::endl;
    QTimer::singleShot(100, [this]() {
        setSelfHover(false);
    });
}

void NodePanel::openToggleChanged(int state) {

}

void NodePanel::updateVisible() {
    setVisible(visibleFromNodeHover || visibleFromSelfHover || node->isSelected());
}
