#pragma once

#include <QtWidgets/QCompleter>

class QLineEdit;

namespace AxiomGui {

    class SpaceCompleter : public QCompleter {
    Q_OBJECT

    public:
        explicit SpaceCompleter(const QStringList &tags, QLineEdit *editor, QObject *parent = nullptr);

        QStringList splitPath(const QString &path) const override;

    private:
        QLineEdit *editor;
    };

}
