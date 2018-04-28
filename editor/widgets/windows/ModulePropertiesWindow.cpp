#include "ModulePropertiesWindow.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtGui/QIcon>
#include <QtCore/QDateTime>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDialogButtonBox>

#include "editor/util.h"
#include "editor/model/Library.h"
#include "../SpaceCompleter.h"

using namespace AxiomGui;

ModulePropertiesWindow::ModulePropertiesWindow(AxiomModel::Library *library) : QDialog(nullptr, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint) {
    setWindowTitle(tr("Add Module"));
    setStyleSheet(AxiomUtil::loadStylesheet(":/SaveModuleWindow.qss"));
    setWindowIcon(QIcon(":/application.ico"));

    setFixedSize(400, 400);

    auto mainLayout = new QGridLayout();

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setMargin(10);

    // todo: put a module preview here?

    auto nameLabel = new QLabel(tr("Name:"), this);
    nameLabel->setObjectName("save-label");
    mainLayout->addWidget(nameLabel, 0, 0);

    nameInput = new QLineEdit("New Module", this);
    mainLayout->addWidget(nameInput, 1, 0);

    auto tagsLabel = new QLabel(tr("Tags: (space-separated)"), this);
    tagsLabel->setObjectName("save-label");
    mainLayout->addWidget(tagsLabel, 2, 0);

    // generate a few random tags
    tagsInput = new QLineEdit(this);
    auto tagList = library->tags();
    auto completer = new SpaceCompleter(tagList, tagsInput, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    std::random_shuffle(tagList.begin(), tagList.end());
    auto tagCount = tagList.size() > 3 ? 3 : tagList.size();

    QString randomTags = "E.g. ";
    for (auto i = 0; i < tagCount; i++) {
        randomTags += tagList[i] + " ";
    }

    tagsInput->setCompleter(completer);
    tagsInput->setPlaceholderText(randomTags);
    mainLayout->addWidget(tagsInput, 3, 0);

    mainLayout->setRowStretch(4, 1);

    auto buttonBox = new QDialogButtonBox();
    auto okButton = buttonBox->addButton(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    auto cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox, 5, 0);

    setLayout(mainLayout);

    connect(okButton, &QPushButton::clicked,
            this, &ModulePropertiesWindow::accept);
    connect(cancelButton, &QPushButton::clicked,
            this, &ModulePropertiesWindow::reject);
}

QString ModulePropertiesWindow::enteredName() const {
    return nameInput->text();
}

QStringList ModulePropertiesWindow::enteredTags() const {
    auto trimmedInput = tagsInput->text().trimmed();
    if (trimmedInput.isEmpty()) return {};
    else return trimmedInput.split(QRegExp("(\\s?,\\s?)|(\\s+)"));
}

void ModulePropertiesWindow::setEnteredName(const QString &name) {
    nameInput->setText(name);
}

void ModulePropertiesWindow::setEnteredTags(const QStringList &list) {
    tagsInput->setText(list.join(' '));
}
