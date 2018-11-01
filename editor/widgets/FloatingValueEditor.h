#pragma once

#include <QtWidgets/QGraphicsProxyWidget>
#include <QtWidgets/QLineEdit>
#include <memory>

namespace AxiomGui {

    class FloatingValueEditor : public QGraphicsProxyWidget {
        Q_OBJECT

    public:
        FloatingValueEditor(QString initialValue, QPointF scenePos, int selectStart = 0, int selectEnd = 0);

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
