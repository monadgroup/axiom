#include "AudioBackend.h"

#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QMessageBox>
#include <ctime>
#include <iostream>
#include <pmmintrin.h>
#include <xmmintrin.h>

#include "../AxiomEditor.h"
#include "../model/ModelRoot.h"
#include "../model/Project.h"
#include "../model/objects/RootSurface.h"
#include "../model/serialize/ProjectSerializer.h"
#include "../util.h"
#include "../widgets/InteractiveImport.h"
#include "../widgets/windows/MainWindow.h"

using namespace AxiomBackend;

GenerateContext::GenerateContext(uint64_t count, AudioBackend *backend)
    : backend(backend), _maxGenerateCount(count),
      runtimeLock(backend->_editor->window()->project()->mainRoot().lockRuntime()) {
    beforeFpuState = _mm_getcsr();
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
}

GenerateContext::~GenerateContext() {
    _mm_setcsr(beforeFpuState);
}

void GenerateContext::generate() {
    backend->generatedSamples++;
    backend->_editor->window()->runtime()->runUpdate();
}

NumValue **AudioBackend::getAudioPortal(size_t portalId) const {
    if (portalId >= portalValues.size()) return nullptr;
    return (NumValue **) &portalValues[portalId];
}

MidiValue **AudioBackend::getMidiPortal(size_t portalId) const {
    if (portalId >= portalValues.size()) return nullptr;
    return (MidiValue **) &portalValues[portalId];
}

const char *AudioBackend::formatNumForm(float testValue, AxiomBackend::NumForm form) {
    return AxiomUtil::getFormUnit(testValue, form);
}

std::string AudioBackend::formatNum(AxiomBackend::NumValue value, bool includeLabel) {
    return AxiomUtil::formatNumForm(value, includeLabel).toStdString();
}

std::string AudioBackend::findDataFile(const std::string &name) {
    return QStandardPaths::locate(QStandardPaths::AppDataLocation, QString::fromStdString(name)).toStdString();
}

std::string AudioBackend::getDataPath() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString();
}

QByteArray AudioBackend::serialize(std::optional<std::function<void(QDataStream &)>> serializeCustomCallback) {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);

    auto project = _editor->window()->project();
    AxiomModel::ProjectSerializer::serialize(project, stream,
                                             [project](QDataStream &stream) { stream << project->linkedFile(); });
    if (serializeCustomCallback) {
        (*serializeCustomCallback)(stream);
    }
    return buffer;
}

void AudioBackend::deserialize(QByteArray *data,
                               std::optional<std::function<void(QDataStream &, uint32_t)>> deserializeCustomCallback) {
    QDataStream stream(data, QIODevice::ReadOnly);
    uint32_t readVersion = 0;
    auto newProject = AxiomModel::ProjectSerializer::deserialize(
        stream, &readVersion,
        [this](AxiomModel::Library *importLibrary) {
            AxiomGui::doInteractiveLibraryImport(_editor->window()->library(), importLibrary);
        },
        [](QDataStream &stream, uint32_t version) {
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
        if (deserializeCustomCallback) {
            (*deserializeCustomCallback)(stream, readVersion);
        }
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

void AudioBackend::clearNotes(size_t portalId) {
    // todo
}

GenerateContext AudioBackend::beginGenerate() {
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

    if (queuedEvents.empty()) {
        return GenerateContext(UINT64_MAX, this);
    } else {
        return GenerateContext(queuedEvents[0].deltaFrames, this);
    }
}

void AudioBackend::previewEvent(AxiomBackend::MidiEvent event) {}

void AudioBackend::automationValueChanged(size_t portalId, AxiomBackend::NumValue value) {}

bool AudioBackend::canFiddleAutomation() const {
    return false;
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

        newPortals.emplace_back(surfacePortal.socketIndex, surfacePortal.id, newType, newValue,
                                surfacePortal.name.toStdString());
    }
    std::sort(newPortals.begin(), newPortals.end());

    // update the value pointers
    portalValues.clear();
    portalValues.reserve(newPortals.size());
    for (const auto &newPortal : newPortals) {
        portalValues.push_back(_editor->window()->runtime()->getPortalPtr(newPortal._key));
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

size_t AudioBackend::internalRemapPortal(uint64_t id) {
    for (size_t portalIndex = 0; portalIndex < currentPortals.size(); portalIndex++) {
        if (currentPortals[portalIndex].id == id) return portalIndex;
    }
    unreachable;
}
