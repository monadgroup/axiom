#pragma once

class QAction;

namespace AxiomGui {

    namespace GlobalActions {
        extern QAction *fileNew;
        extern QAction *fileImportLibrary;
        extern QAction *fileExportLibrary;
        extern QAction *fileOpen;
        extern QAction *fileSave;
        extern QAction *fileSaveAs;
        extern QAction *fileExport;
        extern QAction *fileQuit;
        extern QAction *editUndo;
        extern QAction *editRedo;
        extern QAction *editCut;
        extern QAction *editCopy;
        extern QAction *editPaste;
        extern QAction *editDelete;
        extern QAction *editSelectAll;
        extern QAction *editPreferences;
        extern QAction *helpAbout;

        void setupActions();
    }
}
