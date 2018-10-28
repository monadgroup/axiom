#pragma once

#include <QtWidgets/QGraphicsObject>
#include <QtWidgets/QPlainTextEdit>

#include "common/TrackedObject.h"
#include "editor/compiler/interface/Frontend.h"

class QGraphicsProxyWidget;

namespace AxiomModel {
    struct CustomNodeError;

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

        void codeChanged(const QString &newCode);

        void triggerUpdate();

        void triggerGeometryChange();

        void resizerChanged(QPointF topLeft, QPointF bottomRight);

    signals:

        void resizerSizeChanged(QSizeF newSize);

    private:
        QGraphicsProxyWidget *textProxy;
        QPlainTextEdit *textEditor;
        SyntaxHighlighter *highlighter;
        QString beforeCode;
        bool willBeInErrorState = false;

        void controlTextChanged();

        void compileError(const AxiomModel::CustomNodeError &error);

        void compileSuccess();

        static void moveCursor(QTextCursor &cursor, MaximFrontend::SourcePos pos, QTextCursor::MoveMode mode);
    };
}
