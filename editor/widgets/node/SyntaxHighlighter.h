#pragma once

#include <QMainWindow>
#include <QRegExp>
#include <QString>
#include <QTextEdit>
#include <QtCore/QRegExp>
#include <QtCore/QVector>
#include <QtGui/QSyntaxHighlighter>
#include <QtGui/QTextCharFormat>
//#include <QFont>

namespace AxiomGui {
    class SyntaxHighlighter : public QSyntaxHighlighter {
        Q_OBJECT

    public:
        explicit SyntaxHighlighter(QTextDocument *parent);

    protected:
        void highlightBlock(const QString &text);

    private:
        struct HighlightRule {
            QRegExp pattern;
            QTextCharFormat format;
        };
        QVector<HighlightRule> highlightRules;

        QRegExp commentStartExpression;
        QRegExp commentEndExpression;

        QTextCharFormat functionFormat;
        QTextCharFormat controlFormat;
        QTextCharFormat numberFormat;
        QTextCharFormat typeFormat;
    };
}
