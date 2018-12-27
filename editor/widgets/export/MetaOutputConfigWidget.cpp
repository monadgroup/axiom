#include "MetaOutputConfigWidget.h"

#include <QtWidgets/QFormLayout>

#include "FileBrowserWidget.h"
#include "PortalDefinitionEditorWidget.h"

using namespace AxiomGui;

MetaOutputConfigWidget::MetaOutputConfigWidget(const AxiomModel::Project &project,
                                               const AxiomBackend::AudioConfiguration &configuration) {
    auto layout = new QFormLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    portalEditor = new PortalDefinitionEditorWidget(project, configuration);
    layout->addRow(portalEditor);

    outputBrowser =
        new FileBrowserWidget("Meta Output Location", "Header File (*.h);;Rust Module (*.rs);;JSON File (*.json)");
    layout->addRow("Location:", outputBrowser);
}

void MetaOutputConfigWidget::setInstrumentPrefix(const QString &oldSafePrefix, const QString &newSafePrefix) {
    portalEditor->setInstrumentPrefix(oldSafePrefix, newSafePrefix);
}
