#include "GlobalActions.h"

#include <QtWidgets/QAction>

QAction *makeAction(const QString &text, const QKeySequence &sequence) {
    auto action = new QAction(text);
    action->setShortcut(sequence);
    return action;
}

QAction *makeAction(const QString &text) {
    return new QAction(text);
}

using namespace AxiomGui;

QAction *GlobalActions::fileNew;
QAction *GlobalActions::fileImportLibrary;
QAction *GlobalActions::fileExportLibrary;
QAction *GlobalActions::fileOpen;
QAction *GlobalActions::fileSave;
QAction *GlobalActions::fileSaveAs;
QAction *GlobalActions::fileExport;
QAction *GlobalActions::fileQuit;
QAction *GlobalActions::editUndo;
QAction *GlobalActions::editRedo;
QAction *GlobalActions::editCut;
QAction *GlobalActions::editCopy;
QAction *GlobalActions::editPaste;
QAction *GlobalActions::editDelete;
QAction *GlobalActions::editSelectAll;
QAction *GlobalActions::editPreferences;
QAction *GlobalActions::helpAbout;

void GlobalActions::setupActions() {
    fileNew = makeAction("&New");
    fileImportLibrary = makeAction("&Import Library...");
    fileExportLibrary = makeAction("E&xport Library...");
    fileOpen = makeAction("&Open...", QKeySequence::Open);
    fileSave = makeAction("&Save", QKeySequence::Save);
    fileSaveAs = makeAction("&Save As...", QKeySequence::SaveAs);
    fileExport = makeAction("&Export...");
    fileExport->setEnabled(false);
    fileQuit = makeAction("&Quit", QKeySequence::Quit);

    editUndo = makeAction("&Undo", QKeySequence::Undo);
    editRedo = makeAction("&Redo", QKeySequence::Redo);
    editCut = makeAction("C&ut", QKeySequence::Cut);
    editCopy = makeAction("&Copy", QKeySequence::Copy);
    editPaste = makeAction("&Paste", QKeySequence::Paste);
    editDelete = makeAction("&Delete", QKeySequence::Delete);
    editSelectAll = makeAction("&Select All", QKeySequence::SelectAll);
    editPreferences = makeAction("Pr&eferences...", QKeySequence::Preferences);
    editPreferences->setEnabled(false);

    helpAbout = makeAction("&About", QKeySequence::HelpContents);
}
