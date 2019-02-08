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

bool ObjectOutputConfigWidget::isConfigValid() {
    return !outputBrowser->location().isEmpty();
}

MaximCompiler::ObjectOutputConfig ObjectOutputConfigWidget::buildConfig() {
    auto location = outputBrowser->location();
    MaximFrontend::ObjectFormat format = MaximFrontend::ObjectFormat::OBJECT;
    if (location.endsWith(".bc")) {
        format = MaximFrontend::ObjectFormat::BITCODE;
    } else if (location.endsWith(".ll")) {
        format = MaximFrontend::ObjectFormat::IR;
    } else if (location.endsWith(".s")) {
        format = MaximFrontend::ObjectFormat::ASSEMBLY_LISTING;
    }

    return MaximCompiler::ObjectOutputConfig(format, location);
}
