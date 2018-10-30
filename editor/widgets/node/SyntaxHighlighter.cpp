#include "SyntaxHighlighter.h"

#include <QtCore/QRegularExpression>
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

    QRegularExpression commentExpression;
    QTextCharFormat commentFormat;

    CachedHighlightRules() : commentExpression(R"((/\*)|(\*/))") {
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
        formRule.format.setForeground(QColor(101, 216, 105)); // green
        formRule.format.setFontWeight(QFont::Bold);
        highlightRules.push_back(std::move(formRule));

        commentFormat.setForeground(QColor(121, 121, 121)); // grey
        commentFormat.setFontWeight(QFont::StyleItalic);

        // comment (single line) formatting
        HighlightRule commentRule;
        commentRule.pattern = QRegExp(R"(#[^\n]*)");
        commentRule.format = commentFormat;
        highlightRules.push_back(std::move(commentRule));
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

    // Use block state to highlight multiline comments.
    // Since the language allows multiline comments inside multiline comments (e.g. /* /* */ */), we let the block state
    // be the depth of the comment at the end of the block. That is, if we're not in a comment the state is 0, otherwise
    // it's > 0. We increment it whenever we find an opening token, and decrement it when we find a closing one.
    setCurrentBlockState(0);

    auto enterCount = previousBlockState();
    if (enterCount == -1) enterCount = 0;

    // Increment/decrement the block state to determine the exit count
    auto currentPos = 0;
    auto lastOpenPos = 0;
    auto exitCount = enterCount;
    while (currentPos < text.size()) {
        auto match = cachedRules.commentExpression.match(text, currentPos);
        if (!match.hasMatch()) break;

        // capture group 1 = comment open
        // capture group 2 = comment close
        if (match.capturedLength(1)) {
            if (exitCount == 0) {
                lastOpenPos = match.capturedStart();
            }

            exitCount++;
        } else if (match.capturedLength(2)) {
            exitCount--;

            // if we're out of the comment, highlight this part
            if (exitCount == 0) {
                setFormat(lastOpenPos, match.capturedEnd() - lastOpenPos, cachedRules.commentFormat);
            } else if (exitCount < 0)
                exitCount = 0;
        }

        currentPos = match.capturedEnd();
    }

    // if the exit count isn't zero, we're still in a comment, so we need to highlight everything to the end
    if (exitCount > 0) {
        setFormat(lastOpenPos, text.size() - lastOpenPos, cachedRules.commentFormat);
    }

    setCurrentBlockState(exitCount);
}
