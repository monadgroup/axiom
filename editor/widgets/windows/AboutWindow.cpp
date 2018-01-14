#include "AboutWindow.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtGui/QIcon>

#include "editor/util.h"
#include "editor/resources/resource.h"

using namespace AxiomGui;

AboutWindow::AboutWindow() : QDialog(nullptr,
                                     Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint) {
    setWindowTitle(tr("About " VER_PRODUCTNAME_STR));
    setStyleSheet(AxiomUtil::loadStylesheet(":/AboutWindow.qss"));
    setWindowIcon(QIcon(":/application.ico"));

    setFixedSize(400, 400);

    auto mainLayout = new QGridLayout();

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setMargin(10);

    QPixmap logoImg(":/logo.png");
    auto logoLabel = new QLabel();
    logoLabel->setObjectName("about-logo");
    logoLabel->setPixmap(logoImg);
    mainLayout->addWidget(logoLabel, 0, 0, 1, 2);

    auto descriptLabel = new QLabel(tr(VER_FILEDESCRIPTION_STR " - " VER_LEGALCOPYRIGHT_STR));
    descriptLabel->setObjectName("about-descript");
    mainLayout->addWidget(descriptLabel, 1, 0, 1, 2);

    auto versionLabel = new QLabel(tr("Version:"));
    versionLabel->setObjectName("about-label");
    mainLayout->addWidget(versionLabel, 2, 0);
    mainLayout->addWidget(new QLabel(tr(VER_FILEVERSION_STR)), 2, 1);

    auto authLabel = new QLabel(tr("Author:"));
    authLabel->setObjectName("about-label");
    mainLayout->addWidget(authLabel, 3, 0);
    mainLayout->addWidget(new QLabel(tr("cpdt (mrfishie.com)")), 3, 1);

    auto contribLabel = new QLabel(tr("Contributors:"));
    contribLabel->setObjectName("about-label");
    mainLayout->addWidget(contribLabel, 4, 0);
    auto noneLabel = new QLabel(tr("(None yet)"));
    noneLabel->setObjectName("about-none");
    mainLayout->addWidget(noneLabel, 4, 1);

    mainLayout->setRowStretch(5, 1);
    mainLayout->setColumnStretch(1, 1);

    setLayout(mainLayout);
}
