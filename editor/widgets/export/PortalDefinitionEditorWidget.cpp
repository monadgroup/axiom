#include "PortalDefinitionEditorWidget.h"

#include <QtCore/QStringBuilder>
#include <QtGui/QImage>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

#include "../controls/PortalControlItem.h"
#include "editor/backend/AudioBackend.h"
#include "editor/backend/AudioConfiguration.h"
#include "editor/model/Project.h"
#include "editor/util.h"

using namespace AxiomGui;

static QString getDefaultPortalTypeString(const AxiomBackend::ConfigurationPortal &portal) {
    const char *baseValue;
    switch (portal.value) {
    case AxiomBackend::PortalValue::AUDIO:
        baseValue = "AUDIO";
        break;
    case AxiomBackend::PortalValue::MIDI:
        baseValue = "MIDI";
        break;
    }

    switch (portal.type) {
    case AxiomBackend::PortalType::INPUT:
        return QString(baseValue) + "_IN";
    case AxiomBackend::PortalType::OUTPUT:
        return QString(baseValue) + "_OUT";
    case AxiomBackend::PortalType::AUTOMATION:
        return "AUTOMATION";
    }

    unreachable;
}

static QString generateDefaultDefinitionForPortal(const AxiomBackend::ConfigurationPortal &portal) {
    return "AXIOM_" % getDefaultPortalTypeString(portal) % "_" %
           AxiomUtil::getSafeDefinition(QString::fromStdString(portal.name).toUpper());
}

PortalDefinitionEditorWidget::PortalDefinitionEditorWidget(const AxiomModel::Project &project,
                                                           const AxiomBackend::AudioConfiguration &configuration) {
    auto mainWidget = new QWidget();
    auto layout = new QFormLayout();
    layout->setContentsMargins(2, 2, 2, 2);
    mainWidget->setLayout(layout);
    setWidget(mainWidget);
    setWidgetResizable(true);

    for (const auto &portal : configuration.portals) {
        auto newDefinition = AxiomUtil::ensureDefinitionIsUnique(generateDefaultDefinitionForPortal(portal), usedNames);
        usedNames.insert(newDefinition);
        auto lineEdit = new QLineEdit(newDefinition);
        lineEdits.push_back({newDefinition, lineEdit});

        QImage portalImage(
            ":/icons/" %
            PortalControlItem::iconNameFromType(AxiomModel::Project::controlTypeFromBackendType(portal.type),
                                                AxiomModel::Project::wireTypeFromBackendValue(portal.value)) %
            "-small.png");
        auto imageLabel = new QLabel();
        imageLabel->setPixmap(QPixmap::fromImage(portalImage));

        auto labelLayout = new QHBoxLayout();
        labelLayout->setContentsMargins(0, 0, 0, 0);
        labelLayout->addWidget(imageLabel);

        auto remappedIndex = project.backend()->internalRemapPortal(portal.id);
        labelLayout->addWidget(new QLabel(QString::fromStdString(portal.name) % " (" %
                                          QString::fromStdString(project.backend()->getPortalLabel(remappedIndex)) %
                                          ")"));

        auto labelWidget = new QWidget();
        labelWidget->setLayout(labelLayout);

        layout->addRow(labelWidget, lineEdit);
    }

    for (size_t i = 0; i < lineEdits.size(); i++) {
        connect(lineEdits[i].lineEdit, &QLineEdit::editingFinished, this, [this, i]() { ensureLineSafe(i); });
    }
}

void PortalDefinitionEditorWidget::setInstrumentPrefix(const QString &oldSafePrefix, const QString &newSafePrefix) {
    auto newUpperPrefix = newSafePrefix.toUpper();

    for (size_t i = 0; i < lineEdits.size(); i++) {
        auto lineEdit = lineEdits[i].lineEdit;
        auto currentText = lineEdit->text();
        if (!currentText.startsWith(oldSafePrefix, Qt::CaseInsensitive)) {
            continue;
        }

        auto newText = newUpperPrefix + currentText.mid(oldSafePrefix.size());
        lineEdit->setText(newText);
        ensureLineSafe(i);
    }
}

void PortalDefinitionEditorWidget::ensureLineSafe(size_t index) {
    auto &lineEdit = lineEdits[index];
    usedNames.remove(lineEdit.lastSafeName);

    auto currentText = lineEdit.lineEdit->text();
    auto newDef = AxiomUtil::ensureDefinitionIsUnique(AxiomUtil::getSafeDefinition(currentText), usedNames);
    usedNames.insert(newDef);
    lineEdit.lastSafeName = newDef;

    if (newDef != currentText) {
        lineEdit.lineEdit->setText(newDef);
    }
}
