#include "SaveModuleWindow.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtGui/QIcon>
#include <QtCore/QDateTime>
#include <QLineEdit>

#include "editor/util.h"
#include "../tageditor/TagLineEdit.h"

using namespace AxiomGui;

SaveModuleWindow::SaveModuleWindow() : QDialog(nullptr, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint) {
    setWindowTitle(tr("Add Module"));
    //setStyleSheet(AxiomUtil::loadStylesheet(":/SaveModuleWindow.qss"));
    setWindowIcon(QIcon(":/application.ico"));

    //setFixedSize(300, 400);
    setBaseSize(300, 400);

    auto mainLayout = new QGridLayout();

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setMargin(10);

    // todo: put a module preview here?

    auto nameLabel = new QLabel(tr("Name:"), this);
    nameLabel->setObjectName("save-name");
    mainLayout->addWidget(nameLabel, 0, 0);

    auto nameInput = new QLineEdit("New Module", this);
    mainLayout->addWidget(nameInput, 0, 1);

    auto tagsLabel = new QLabel(tr("Tags:"), this);
    tagsLabel->setObjectName("save-tags");
    mainLayout->addWidget(tagsLabel, 1, 0);

    auto tagsInput = new TagLineEdit(this);
    mainLayout->addWidget(tagsInput, 1, 1);

    setLayout(mainLayout);
}
