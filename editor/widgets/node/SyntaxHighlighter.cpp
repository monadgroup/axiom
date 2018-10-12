#include "SyntaxHighlighter.h"

#include <QtCore/QStringBuilder>

#include "editor/compiler/interface/FunctionTable.h"

using namespace AxiomGui;

class CachedHighlightRules {
public:
    struct HighlightRule {
        QRegExp pattern;
        QTextCharFormat format;
    };

    std::vector<HighlightRule> highlightRules;

    CachedHighlightRules() {
        // control formatting
        HighlightRule controlRule;
        controlRule.pattern = QRegExp(R"(:(num|num\[\]|midi|midi\[\]|graph|roll|scope)\b)");
        controlRule.format.setForeground(QColor(234, 106, 89)); // red
        controlRule.format.setFontWeight(QFont::Bold);
        highlightRules.push_back(std::move(controlRule));

        // function formatting
        // build a regex based on the list of function names
        QStringList list;
        auto functionTableSize = MaximCompiler::FunctionTable::size();
        for (size_t functionIndex = 0; functionIndex < functionTableSize; functionIndex++) {
            list.append(MaximCompiler::FunctionTable::find(functionIndex));
        }
        HighlightRule functionRule;
        functionRule.pattern = QRegExp(R"(\b()" % list.join('|') % R"()\b)");
        functionRule.format.setForeground(QColor(123, 181, 247)); // blue
        functionRule.format.setFontWeight(QFont::Bold);
        highlightRules.push_back(std::move(functionRule));

        // number formatting
        HighlightRule numberRule;
        numberRule.pattern =
            QRegExp(R"(\b([0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?)\s*[kmgt]?\s*(hz|db|q|s|b)?\b)", Qt::CaseInsensitive);
        numberRule.format.setForeground(QColor(193, 127, 226)); // purple
        numberRule.format.setFontWeight(QFont::Bold);
        highlightRules.push_back(std::move(numberRule));

        // form formatting
        HighlightRule formRule;
        formRule.pattern = QRegExp(R"(\b(none|control|osc|note|freq|beats|secs|samples|db|amp|q)\b)");
        formRule.format.setForeground(QColor(101, 216, 105));
        formRule.format.setFontWeight(QFont::Bold);
        highlightRules.push_back(std::move(formRule));
    }
};
CachedHighlightRules cachedRules;

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    for (const auto &highlightRule : cachedRules.highlightRules) {
        QRegExp expression(highlightRule.pattern);
        auto index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, highlightRule.format);
            index = expression.indexIn(text, index + length);
        }
    }
}
