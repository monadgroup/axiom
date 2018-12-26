#pragma once

#include <QtWidgets/QWidget>

class QLineEdit;

namespace AxiomGui {

    class FileBrowserWidget : public QWidget {
        Q_OBJECT

    public:
        FileBrowserWidget(QString caption, QString filter, QString directory = QString());

        void setLocation(const QString &location);
        QString location();

    private slots:
        void browseForFile();

    private:
        QString caption;
        QString filter;
        QString directory;
        QLineEdit *locationEdit;
    };
}
