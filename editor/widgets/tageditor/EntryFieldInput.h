#pragma once

#include <QtWidgets/QLineEdit>

namespace AxiomGui {

    class EntryFieldInput : public QLineEdit {
        Q_OBJECT

    public:

        EntryFieldInput(QWidget *parent = nullptr);

    signals:

        void submitted(const QString &text);

        void deletePrevious();

        void deleteNext();

        void focusPrevious();

        void focusNext();

    protected:

        void keyPressEvent(QKeyEvent *event) override;

    private slots:

        void resizeToFit();
    };

}
