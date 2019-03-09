#pragma once

#include <QtWidgets/QDialog>

class QLineEdit;

namespace AxiomModel {
    class Library;
}

namespace AxiomGui {

    class ModulePropertiesWindow : public QDialog {
        Q_OBJECT

    public:
        ModulePropertiesWindow(AxiomModel::Library *library, const char *caption);

        QString enteredName() const;

        QStringList enteredTags() const;

    public slots:

        void setEnteredName(const QString &name);

        void setEnteredTags(const QStringList &list);

    private:
        QLineEdit *nameInput;
        QLineEdit *tagsInput;
    };
}
