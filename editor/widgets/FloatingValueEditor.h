#pragma once

#include <QtWidgets/QGraphicsProxyWidget>

class QLineEdit;

namespace AxiomGui {

    class FloatingValueEditor : public QGraphicsProxyWidget {
        Q_OBJECT

    public:

        explicit FloatingValueEditor(QString initialValue, QPointF scenePos);

    signals:

        void valueSubmitted(QString value);

    private slots:

        void editingFinished();

    private:

        QLineEdit *editor;
    };

}
