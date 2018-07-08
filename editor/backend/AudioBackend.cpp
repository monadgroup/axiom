#include "AudioBackend.h"

#include <QtWidgets/QMessageBox>

#include "../AxiomEditor.h"
#include "../model/Project.h"
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
    _editor->window()->project()->serialize(stream);
    return std::move(buffer);
}

void AudioBackend::deserialize(QByteArray *data) {
    QDataStream stream(data, QIODevice::ReadOnly);
    uint32_t readVersion = 0;
    auto newProject = AxiomModel::Project::deserialize(stream, &readVersion);

    if (!newProject) {
        if (readVersion) {
            QMessageBox(QMessageBox::Critical, "Failed to load project",
                        "This project was created with an incompatible version of Axiom.\n\n"
                        "Expected version: between " +
                            QString::number(AxiomModel::Project::minSchemaVersion) + " and " +
                            QString::number(AxiomModel::Project::schemaVersion) +
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
    _editor->runtime()->setBpm(bpm);
}

void AudioBackend::setSampleRate(float sampleRate) {
    _editor->runtime()->setSampleRate(sampleRate);
}

void AudioBackend::queueMidiEvent(uint64_t deltaFrames, size_t portalId, AxiomBackend::MidiEvent event) {
    queuedEvents.push_back({deltaFrames, portalId, event});
}

void AudioBackend::clearMidi(size_t portalId) {
    (*getMidiPortal(portalId))->count = 0;
}

void AudioBackend::clearNotes(size_t portalId) {}

uint64_t AudioBackend::beginGenerate() {
    // decrement all deltaFrames from last time
    for (auto &event : queuedEvents) {
        event.deltaFrames -= generatedSamples;

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
    _editor->runtime()->runUpdate();
}

AudioConfiguration AudioBackend::createDefaultConfiguration() {
    return AudioConfiguration({ConfigurationPortal(PortalType::INPUT, PortalValue::MIDI, "Keyboard"),
                               ConfigurationPortal(PortalType::OUTPUT, PortalValue::AUDIO, "Speakers")});
}
