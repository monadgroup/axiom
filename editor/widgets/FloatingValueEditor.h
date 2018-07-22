#pragma once

#include <QtWidgets/QGraphicsProxyWidget>

class QLineEdit;

namespace AxiomGui {

    class FloatingValueEditor : public QGraphicsProxyWidget {
        Q_OBJECT

    public:
        explicit FloatingValueEditor(QString initialValue, QPointF scenePos);

    protected:
        bool eventFilter(QObject *object, QEvent *event) override;

    signals:

        void valueSubmitted(QString value);

    private slots:

        void editingFinished();

    private:
        QLineEdit *editor;
    };
}
