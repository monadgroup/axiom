#include "AudioBackend.h"

#include <QtCore/QFileInfo>
#include <QtWidgets/QMessageBox>

#include "../AxiomEditor.h"
#include "../model/ModelRoot.h"
#include "../model/Project.h"
#include "../model/objects/RootSurface.h"
#include "../model/serialize/ProjectSerializer.h"
#include "../resources/resource.h"
#include "../widgets/windows/MainWindow.h"

using namespace AxiomBackend;

const char *AxiomBackend::PRODUCT_VERSION = VER_PRODUCTVERSION_STR;
const char *AxiomBackend::COMPANY_NAME = VER_COMPANYNAME_STR;
const char *AxiomBackend::FILE_DESCRIPTION = VER_FILEDESCRIPTION_STR;
const char *AxiomBackend::INTERNAL_NAME = VER_INTERNALNAME_STR;
const char *AxiomBackend::LEGAL_COPYRIGHT = VER_LEGALCOPYRIGHT_STR;
const char *AxiomBackend::LEGAL_TRADEMARKS = VER_LEGALTRADEMARKS1_STR;
const char *AxiomBackend::PRODUCT_NAME = VER_PRODUCTNAME_STR;

NumValue **AudioBackend::getAudioPortal(size_t portalId) const {
    if (portalId >= portalValues.size()) return nullptr;
    return (NumValue **) &portalValues[portalId];
}

MidiValue **AudioBackend::getMidiPortal(size_t portalId) const {
    if (portalId >= portalValues.size()) return nullptr;
    return (MidiValue **) &portalValues[portalId];
}

const char *AudioBackend::formatNumForm(AxiomBackend::NumForm form) const {
    switch (form) {
    case AxiomModel::FormType::NONE:
    case AxiomModel::FormType::CONTROL:
    case AxiomModel::FormType::OSCILLATOR:
    case AxiomModel::FormType::AMPLITUDE:
    case AxiomModel::FormType::Q:
    case AxiomModel::FormType::NOTE:
    case AxiomModel::FormType::SAMPLES:
    default:
        return "";
    case AxiomModel::FormType::FREQUENCY:
        return " Hz";
    case AxiomModel::FormType::BEATS:
        return " beats";
    case AxiomModel::FormType::SECONDS:
        return " s";
    case AxiomModel::FormType::DB:
        return " dB";
    }
}

std::string AudioBackend::formatNum(AxiomBackend::NumValue value, bool includeLabel) const {
    // todo: refactor this with the code from NumControlItem
    return "";
}

QByteArray AudioBackend::serialize() {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);

    auto project = _editor->window()->project();
    AxiomModel::ProjectSerializer::serialize(project, stream,
                                             [project](QDataStream &stream) { stream << project->linkedFile(); });
    return buffer;
}

void AudioBackend::deserialize(QByteArray *data) {
    QDataStream stream(data, QIODevice::ReadOnly);
    uint32_t readVersion = 0;
    auto newProject =
        AxiomModel::ProjectSerializer::deserialize(stream, &readVersion, [](QDataStream &stream, uint32_t version) {
            QString linkedFile;
            if (version >= 5) {
                stream >> linkedFile;

                // only link to the file if it actually exists
                QFileInfo linkedFileInfo(linkedFile);
                if (!linkedFileInfo.exists() || !linkedFileInfo.isFile()) {
                    linkedFile = "";
                }
            }
            return linkedFile;
        });

    if (!newProject) {
        if (readVersion) {
            QMessageBox(QMessageBox::Critical, "Failed to load project",
                        "This project was created with an incompatible version of Axiom.\n\n"
                        "Expected version: between " +
                            QString::number(AxiomModel::ProjectSerializer::minSchemaVersion) + " and " +
                            QString::number(AxiomModel::ProjectSerializer::schemaVersion) +
                            ", actual version: " + QString::number(readVersion) + ".",
                        QMessageBox::Ok)
                .exec();
        } else {
            QMessageBox(QMessageBox::Critical, "Failed to load project",
                        "This project is an invalid project file (bad magic header).\n"
                        "Maybe it's corrupt?",
                        QMessageBox::Ok)
                .exec();
        }
    } else {
        _editor->window()->setProject(std::move(newProject));
    }
}

void AudioBackend::setBpm(float bpm) {
    _editor->window()->runtime()->setBpm(bpm);
}

void AudioBackend::setSampleRate(float sampleRate) {
    _editor->window()->runtime()->setSampleRate(sampleRate);
}

void AudioBackend::queueMidiEvent(uint64_t deltaFrames, size_t portalId, AxiomBackend::MidiEvent event) {
    queuedEvents.push_back({deltaFrames, portalId, event});
}

void AudioBackend::clearMidi(size_t portalId) {
    (*getMidiPortal(portalId))->count = 0;
}

void AudioBackend::clearNotes(size_t portalId) {}

std::lock_guard<std::mutex> AudioBackend::lockRuntime() {
    return _editor->window()->project()->mainRoot().lockRuntime();
}

uint64_t AudioBackend::beginGenerate() {
    // decrement all deltaFrames from last time
    for (auto &event : queuedEvents) {
        if (event.deltaFrames > generatedSamples) {
            event.deltaFrames -= generatedSamples;
        } else {
            event.deltaFrames = 0;
        }

        if (event.deltaFrames == 0) {
            (*getMidiPortal(event.portalId))->pushEvent(event.event);

            // note: deque doesn't invalidate iterators when removing items from start or end, so this is safe to do.
            queuedEvents.pop_front();
        }
    }
    generatedSamples = 0;

    // return number of samples to next event
    if (queuedEvents.empty()) {
        return UINT64_MAX;
    } else {
        return queuedEvents[0].deltaFrames;
    }
}

void AudioBackend::generate() {
    generatedSamples++;
    _editor->window()->runtime()->runUpdate();
}

DefaultConfiguration AudioBackend::createDefaultConfiguration() {
    return DefaultConfiguration({DefaultPortal(PortalType::INPUT, PortalValue::MIDI, "Keyboard"),
                                 DefaultPortal(PortalType::OUTPUT, PortalValue::AUDIO, "Speakers")});
}

void AudioBackend::internalUpdateConfiguration() {
    std::vector<ConfigurationPortal> newPortals;
    assert(_editor->window()->project()->rootSurface()->compileMeta());
    auto &compileMeta = *_editor->window()->project()->rootSurface()->compileMeta();

    for (const auto &surfacePortal : compileMeta.portals) {
        PortalType newType;
        switch (surfacePortal.portalType) {
        case AxiomModel::PortalControl::PortalType::INPUT:
            newType = PortalType::INPUT;
            break;
        case AxiomModel::PortalControl::PortalType::OUTPUT:
            newType = PortalType::OUTPUT;
            break;
        case AxiomModel::PortalControl::PortalType::AUTOMATION:
            newType = PortalType::AUTOMATION;
            break;
        }

        PortalValue newValue;
        switch (surfacePortal.valueType) {
        case AxiomModel::ConnectionWire::WireType::NUM:
            newValue = PortalValue::AUDIO;
            break;
        case AxiomModel::ConnectionWire::WireType::MIDI:
            newValue = PortalValue::MIDI;
            break;
        }

        newPortals.emplace_back(surfacePortal.id, newType, newValue, surfacePortal.name.toStdString());
    }
    std::sort(newPortals.begin(), newPortals.end());

    // update the value pointers
    portalValues.clear();
    portalValues.reserve(newPortals.size());
    for (size_t i = 0; i < newPortals.size(); i++) {
        portalValues.push_back(_editor->window()->runtime()->getPortalPtr(i));
    }

    // no point continuing if the portals are the same
    if (hasCurrent && newPortals == currentPortals) {
        return;
    }

    AudioConfiguration currentConfiguration(std::move(newPortals));
    handleConfigurationChange(currentConfiguration);

    currentPortals = std::move(currentConfiguration.portals);
    hasCurrent = true;
}
