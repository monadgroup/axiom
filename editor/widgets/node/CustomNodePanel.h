#pragma once

#include <QtWidgets/QGraphicsObject>
#include <QtWidgets/QPlainTextEdit>

#include "compiler/runtime/ErrorLog.h"
#include "common/Hookable.h"

class QGraphicsProxyWidget;

namespace AxiomModel {

    class CustomNode;

}

namespace AxiomGui {

    class CustomNodePanel : public QGraphicsObject, public AxiomCommon::Hookable {
    Q_OBJECT

    public:

        AxiomModel::CustomNode *node;

        explicit CustomNodePanel(AxiomModel::CustomNode *node);

        QRectF boundingRect() const override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    private slots:

        void updateSize();

        void setOpen(bool open);

        void setError(const MaximRuntime::ErrorLog &log);

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
        bool hasErrors = false;
        bool showingErrors = false;

        bool eventFilter(QObject *object, QEvent *event) override;

        void controlTextChanged();

        static void moveCursor(QTextCursor &cursor, SourcePos pos, QTextCursor::MoveMode mode);

    };

}
