#pragma once

#include <set>
#include <QtWidgets/QScrollArea>

class QCompleter;

class QHBoxLayout;

namespace AxiomGui {

    class EntryFieldItem;

    class TagLineEdit : public QScrollArea {
        Q_OBJECT

    public:
        TagLineEdit(QWidget *parent = nullptr);

        void setCompleter(QCompleter *completer);

    public slots:

        void addTag(const QString &name);

        void removeTag(const QString &name);

    signals:

        void tagAdded(const QString &name);

        void tagRemoved(const QString &name);

    private:

        QHBoxLayout *layout;

        EntryFieldItem *firstItem;

    };

}
