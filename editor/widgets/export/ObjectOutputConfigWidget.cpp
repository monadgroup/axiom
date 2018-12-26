#include "ObjectOutputConfigWidget.h"

#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>

#include "FileBrowserWidget.h"

using namespace AxiomGui;

ObjectOutputConfigWidget::ObjectOutputConfigWidget() {
    auto layout = new QFormLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    outputBrowser =
        new FileBrowserWidget("Object Output Location",
                              "Object File (*.o);;LLVM Bitcode File (*.bc);;LLVM IR (*.ll);;Assembly Listing (*.s)");
    layout->addRow("Location:", outputBrowser);
}
