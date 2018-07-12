#include "SpaceCompleter.h"

#include <QtWidgets/QLineEdit>

using namespace AxiomGui;

QRegExp spaceRegex("\\s");

SpaceCompleter::SpaceCompleter(const QStringList &tags, QLineEdit *editor, QObject *parent)
    : QCompleter(tags, parent), editor(editor) {}

QStringList SpaceCompleter::splitPath(const QString &path) const {
    auto currentWordStart = qMax(0, path.lastIndexOf(spaceRegex, editor->cursorPosition()) + 1);
    auto currentWordEnd = path.indexOf(spaceRegex, currentWordStart);
    if (currentWordEnd == -1) currentWordEnd = path.size();

    return {path.mid(currentWordStart, currentWordEnd - currentWordStart)};
}
