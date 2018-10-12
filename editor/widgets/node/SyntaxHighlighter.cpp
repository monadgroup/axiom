#include "SyntaxHighlighter.h"

using namespace AxiomGui;

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    // Syntax Highlighting Rules for Controls
    HighlightRule controlRule;

    controlFormat.setForeground(QColor(234, 106, 89)); // Red
    controlFormat.setFontWeight(QFont::Bold);
    QStringList controlKeywordPatterns;
    controlKeywordPatterns << "\\bnum\\b"
                           << "\\bnum[]\\b"
                           << "\\bmidi\\b"
                           << "b\\bmidi[]\\b"
                           << "\\bgraph\\b"
                           << "\\broll\\b"
                           << "\\bscope\\b";
    foreach (const QString &pattern, controlKeywordPatterns) {
        controlRule.pattern = QRegExp(pattern);
        controlRule.format = controlFormat;
        highlightRules.append(controlRule);
    }

    // Syntax Highlighting Rules for Functions
    HighlightRule functionRule;

    functionFormat.setForeground(QColor(123, 181, 247)); // Blue
    functionFormat.setFontWeight(QFont::Bold);
    QStringList functionKeywordPatterns;
    functionKeywordPatterns << "\\bmix\\b"
                            << "\\bclamp\\b"
                            << "\\baccum\\b"
                            << "b\\bhold\\b"
                            << "\\bpan\\b"
                            << "\\bamplitude\\b"
                            << "\\bsawOsc\\b";
    foreach (const QString &pattern, functionKeywordPatterns) {
        functionRule.pattern = QRegExp(pattern);
        functionRule.format = functionFormat;
        highlightRules.append(functionRule);
    }

    // Syntax Highlighting Rules for Numbers and Units
    HighlightRule numberRule;

    numberFormat.setForeground(QColor(193, 127, 226)); // Purple
    numberFormat.setFontWeight(QFont::Bold);
    QStringList numberKeywordPatterns;
    numberKeywordPatterns << "\\b1\\b"
                          << "\\b2\\b"
                          << "\\b3\\b"
                          << "b\\b4\\b"
                          << "\\b5\\b"
                          << "\\b6\\b"
                          << "\\b7\\b"
                          << "\\b8\\b"
                          << "\\b9\\b"
                          << "\\b0\\b"
                          << "\\b1k\\b"
                          << "\\b2k\\b"
                          << "\\b3k\\b"
                          << "b\\b4k\\b"
                          << "\\b5k\\b"
                          << "\\b6k\\b"
                          << "\\b7k\\b"
                          << "\\b8k\\b"
                          << "\\b9k\\b"
                          << "\\b1m\\b"
                          << "\\b2m\\b"
                          << "\\b3m\\b"
                          << "b\\b4m\\b"
                          << "\\b5m\\b"
                          << "\\b6m\\b"
                          << "\\b7m\\b"
                          << "\\b8m\\b"
                          << "\\b9m\\b"
                          << "\\b1kHz\\b"
                          << "\\b2kHz\\b"
                          << "\\b3kHz\\b"
                          << "b\\b4kHz\\b"
                          << "\\b5kHz\\b"
                          << "\\b6kHz\\b"
                          << "\\b7kHz\\b"
                          << "\\b8kHz\\b"
                          << "\\b9kHz\\b"
                          << "\\b1mHz\\b"
                          << "\\b2mHz\\b"
                          << "\\b3mHz\\b"
                          << "b\\b4mHz\\b"
                          << "\\b5mHz\\b"
                          << "\\b6mHz\\b"
                          << "\\b7mHz\\b"
                          << "\\b8mHz\\b"
                          << "\\b9mHz\\b";
    foreach (const QString &pattern, numberKeywordPatterns) {
        numberRule.pattern = QRegExp(pattern);
        numberRule.format = numberFormat;
        highlightRules.append(numberRule);
    }

    // Syntax Highlighting Rules for Types
    HighlightRule typeRule;

    typeFormat.setForeground(QColor(101, 216, 105)); // Green
    typeFormat.setFontWeight(QFont::Bold);
    QStringList typeKeywordPatterns;
    typeKeywordPatterns << "\\bfreq\\b"
                        << "\\bdB\\b"
                        << "\\bbeats\\b"
                        << "b\\bcontrol\\b"
                        << "\\bamp\\b"
                        << "\\bnone\\b"
                        << "\\bnote\\b"
                        << "\\bosc\\b"
                        << "\\bq\\b"
                        << "\\bsamples\\b"
                        << "\\bsecs\\b";
    foreach (const QString &pattern, typeKeywordPatterns) {
        typeRule.pattern = QRegExp(pattern);
        typeRule.format = typeFormat;
        highlightRules.append(typeRule);
    }
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    foreach (const HighlightRule &controlRule, highlightRules) {
        QRegExp expression(controlRule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, controlRule.format);
            index = expression.indexIn(text, index + length);
        }
    }

    foreach (const HighlightRule &functionRule, highlightRules) {
        QRegExp expression(functionRule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, functionRule.format);
            index = expression.indexIn(text, index + length);
        }
    }

    foreach (const HighlightRule &numberRule, highlightRules) {
        QRegExp expression(numberRule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, numberRule.format);
            index = expression.indexIn(text, index + length);
        }
    }

    foreach (const HighlightRule &typeRule, highlightRules) {
        QRegExp expression(typeRule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, typeRule.format);
            index = expression.indexIn(text, index + length);
        }
    }
}
