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
    auto logoLabel = new QLabel(this);
    logoLabel->setObjectName("about-logo");
    logoLabel->setPixmap(logoImg);
    mainLayout->addWidget(logoLabel, 0, 0, 1, 2);

    auto descriptLabel = new QLabel(tr(VER_FILEDESCRIPTION_STR " - " VER_LEGALCOPYRIGHT_STR), this);
    descriptLabel->setObjectName("about-descript");
    mainLayout->addWidget(descriptLabel, 1, 0, 1, 2);

    auto versionLabel = new QLabel(tr("Version:"), this);
    versionLabel->setObjectName("about-label");
    mainLayout->addWidget(versionLabel, 2, 0);
    mainLayout->addWidget(new QLabel(tr(VER_FILEVERSION_STR) + ", built " + __DATE__ " " __TIME__, this), 2, 1);

    auto authLabel = new QLabel(tr("Author:"), this);
    authLabel->setObjectName("about-label");
    mainLayout->addWidget(authLabel, 3, 0);
    mainLayout->addWidget(new QLabel(tr("cpdt (github.com/cpdt)"), this), 3, 1);

    auto contribLabel = new QLabel(tr("Contributors:"), this);
    contribLabel->setObjectName("about-label");
    mainLayout->addWidget(contribLabel, 4, 0);
    mainLayout->addWidget(new QLabel(tr("PoroCYon"), this), 4, 1);
    mainLayout->addWidget(new QLabel(tr("Firewood"), this), 5, 1);
    auto couldBeYouLabel = new QLabel(
        tr("You could be here too! Help out by contributing to the project on Github: github.com/monadgroup/axiom"),
        this);
    couldBeYouLabel->setWordWrap(true);
    couldBeYouLabel->setObjectName("about-none");
    mainLayout->addWidget(couldBeYouLabel, 6, 1);

    mainLayout->setRowStretch(7, 1);
    mainLayout->setColumnStretch(1, 1);

    setLayout(mainLayout);
}
