#pragma once

#include <QtGui/QSyntaxHighlighter>
#include <QtCore/QRegularExpression>

namespace AxiomGui {

    class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

    public:
        explicit SyntaxHighlighter(QTextDocument *parent);

    protected:
        void highlightBlock(const QString &text) override;

    private:
        struct HighlightRule {
            QRegularExpression pattern;
            QTextCharFormat format;
        };
        QVector<HighlightRule> highlightRules;

        QRegularExpression commentStartExpression;
        QRegularExpression commentEndExpression;

        QTextCharFormat keywordFormat;
        QTextCharFormat classFormat;
        QTextCharFormat singleLineCommentFormat;
        QTextCharFormat multiLineCommentFormat;
        QTextCharFormat quotationFormat;
        QTextCharFormat functionFormat;
    };

}
