#pragma once

#include <QtWidgets/QDialog>

class QLineEdit;

namespace AxiomModel {
    class Library;
}

namespace AxiomGui {

    class SaveModuleWindow : public QDialog {
        Q_OBJECT

    public:
        SaveModuleWindow(AxiomModel::Library *library);

        QString enteredName() const;

        QStringList enteredTags() const;

    private:

        QLineEdit *nameInput;
        QLineEdit *tagsInput;
    };

}
