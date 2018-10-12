#pragma once

#include <QtGui/QSyntaxHighlighter>

namespace AxiomGui {
    class SyntaxHighlighter : public QSyntaxHighlighter {
        Q_OBJECT

    public:
        explicit SyntaxHighlighter(QTextDocument *parent);

    protected:
        void highlightBlock(const QString &text) override;
    };
}
