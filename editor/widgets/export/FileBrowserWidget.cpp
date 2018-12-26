#include "FileBrowserWidget.h"

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

using namespace AxiomGui;

FileBrowserWidget::FileBrowserWidget(QString caption, QString filter, QString directory)
    : caption(std::move(caption)), filter(std::move(filter)), directory(std::move(directory)) {
    auto layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    locationEdit = new QLineEdit();
    layout->addWidget(locationEdit, 1);

    auto pushButton = new QPushButton("...");
    pushButton->setFixedWidth(30);
    layout->addWidget(pushButton, 0);

    connect(pushButton, &QPushButton::clicked, this, &FileBrowserWidget::browseForFile);
}

void FileBrowserWidget::setLocation(const QString &location) {
    locationEdit->setText(location);
}

QString FileBrowserWidget::location() {
    return locationEdit->text();
}

void FileBrowserWidget::browseForFile() {
    QFileDialog saveDialog(this, caption, directory, filter);
    saveDialog.setAcceptMode(QFileDialog::AcceptSave);
    auto currentLocation = location();
    if (!currentLocation.isNull()) {
        saveDialog.selectFile(currentLocation);
    }

    if (saveDialog.exec()) {
        setLocation(saveDialog.selectedFiles()[0]);
    }
}
