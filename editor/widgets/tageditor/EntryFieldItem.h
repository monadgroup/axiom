#pragma once

#include <QtWidgets/QWidget>

class QBoxLayout;

namespace AxiomGui {

    class EntryFieldBox;
    class EntryFieldInput;

    class EntryFieldItem : public QWidget {
        Q_OBJECT

    public:
        EntryFieldItem *previous = nullptr;
        EntryFieldItem *next = nullptr;

        EntryFieldBox *box = nullptr;
        EntryFieldInput *input;

        EntryFieldItem(const QString &name, QBoxLayout *parentLayout, QWidget *parent = nullptr);

    private slots:

        void inputSubmitted(const QString &text);

        void previousDeleted();

        void nextDeleted();

        void previousFocused();

        void nextFocused();

    private:
        QBoxLayout *parentLayout;
    };

}
