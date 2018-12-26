#include "PortalDefinitionEditorWidget.h"

#include <QtCore/QRegularExpression>
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

static QRegularExpression nonSafeCharacters(R"(_?[^0-9A-Za-z_]+_?)");
static QRegularExpression endNumber(R"(\d+$)");

static QString getSafeDefinition(QString definition) {
    // replace characters that are not a-zA-Z_ to underscores
    definition.replace(nonSafeCharacters, "_");
    return definition;
}

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
           getSafeDefinition(QString::fromStdString(portal.name).toUpper());
}

static QString ensureDefinitionIsUnique(QString definition, QSet<QString> &usedNames) {
    if (usedNames.find(definition) == usedNames.end()) {
        return definition;
    }

    auto suffix = 2;

    // if it already ends in a number, use that for the start suffix
    auto endNumberMatch = endNumber.match(definition);
    if (endNumberMatch.hasMatch()) {
        suffix = endNumberMatch.captured(0).toInt() + 1;
        definition = definition.left(endNumberMatch.capturedStart(0));
    } else {
        definition += "_";
    }

    while (true) {
        auto combinedDef = definition + QString::number(suffix);
        if (usedNames.find(combinedDef) == usedNames.end()) {
            return combinedDef;
        }
    }
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
        auto newDefinition = ensureDefinitionIsUnique(generateDefaultDefinitionForPortal(portal), usedNames);
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

void PortalDefinitionEditorWidget::ensureLineSafe(size_t index) {
    auto &lineEdit = lineEdits[index];
    usedNames.remove(lineEdit.lastSafeName);

    auto currentText = lineEdit.lineEdit->text();
    auto newDef = ensureDefinitionIsUnique(getSafeDefinition(currentText), usedNames);
    usedNames.insert(newDef);
    lineEdit.lastSafeName = newDef;

    if (newDef != currentText) {
        lineEdit.lineEdit->setText(newDef);
    }
}
