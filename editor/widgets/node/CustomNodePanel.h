#pragma once

#include <QtWidgets/QGraphicsObject>
#include <QtWidgets/QPlainTextEdit>

#include "common/TrackedObject.h"

class QGraphicsProxyWidget;

namespace AxiomModel {

    class CustomNode;
}

namespace AxiomGui {

    class SyntaxHighlighter;

    class CustomNodePanel : public QGraphicsObject, public AxiomCommon::TrackedObject {
        Q_OBJECT

    public:
        AxiomModel::CustomNode *node;

        explicit CustomNodePanel(AxiomModel::CustomNode *node);

        QRectF boundingRect() const override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    protected:
        bool eventFilter(QObject *object, QEvent *event) override;

    private slots:

        void updateSize();

        void setOpen(bool open);

        void clearError();

        void codeChanged(const QString &newCode);

        void triggerUpdate();

        void triggerGeometryChange();

        void resizerChanged(QPointF topLeft, QPointF bottomRight);

        void compileFinished();

    signals:

        void resizerSizeChanged(QSizeF newSize);

    private:
        QGraphicsProxyWidget *textProxy;
        QPlainTextEdit *textEditor;
        SyntaxHighlighter *highlighter;
        QString beforeCode;
        // bool hasErrors = false;
        // bool showingErrors = false;

        void controlTextChanged();
    };
}
