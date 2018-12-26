#include "InteractiveImport.h"

#include <QtCore/QString>
#include <QtCore/QStringBuilder>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

#include "../model/Library.h"
#include "../model/LibraryEntry.h"
#include "../util.h"

using namespace AxiomGui;

void AxiomGui::doInteractiveLibraryImport(AxiomModel::Library *targetLibrary, AxiomModel::Library *sourceLibrary) {
    targetLibrary->import(sourceLibrary, [](AxiomModel::LibraryEntry *oldEntry, AxiomModel::LibraryEntry *newEntry) {
        auto currentNewer = oldEntry->modificationDateTime() > newEntry->modificationDateTime();

        QMessageBox msgBox(
            QMessageBox::Warning, "Module import conflict",
            QString("Heads up! One of the modules in the imported library is conflicting with one you already had.\n\n"
                    "Original module (") %
                (currentNewer ? "newer" : "older") %
                ")\n"
                "Name: " %
                oldEntry->name() %
                "\n"
                "Last edit: " %
                oldEntry->modificationDateTime().toLocalTime().toString() %
                "\n\n"
                "Imported module (" %
                (currentNewer ? "older" : "newer") %
                ")\n"
                "Name: " %
                newEntry->name() %
                "\n"
                "Last edit: " %
                newEntry->modificationDateTime().toLocalTime().toString() %
                "\n\n"
                "Would you like to keep the current module, imported one, or both?");
        auto cancelBtn = msgBox.addButton(QMessageBox::Cancel);
        auto currentBtn = msgBox.addButton("Current", QMessageBox::ActionRole);
        auto importedBtn = msgBox.addButton("Imported", QMessageBox::ActionRole);
        auto bothBtn = msgBox.addButton("Both", QMessageBox::ActionRole);
        msgBox.setDefaultButton(importedBtn);
        AxiomUtil::showMessageBox(msgBox);

        if (msgBox.clickedButton() == cancelBtn)
            return AxiomModel::Library::ConflictResolution::CANCEL;
        else if (msgBox.clickedButton() == currentBtn)
            return AxiomModel::Library::ConflictResolution::KEEP_OLD;
        else if (msgBox.clickedButton() == importedBtn)
            return AxiomModel::Library::ConflictResolution::KEEP_NEW;
        else if (msgBox.clickedButton() == bothBtn)
            return AxiomModel::Library::ConflictResolution::KEEP_BOTH;
        else
            unreachable;
    });
}
