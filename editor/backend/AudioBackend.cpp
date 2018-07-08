#include "AudioBackend.h"
#include "../resources/resource.h"

using namespace AxiomBackend;

const char *AxiomBackend::PRODUCT_VERSION = VER_PRODUCTVERSION_STR;
const char *AxiomBackend::COMPANY_NAME = VER_COMPANYNAME_STR;
const char *AxiomBackend::FILE_DESCRIPTION = VER_FILEDESCRIPTION_STR;
const char *AxiomBackend::INTERNAL_NAME = VER_INTERNALNAME_STR;
const char *AxiomBackend::LEGAL_COPYRIGHT = VER_LEGALCOPYRIGHT_STR;
const char *AxiomBackend::LEGAL_TRADEMARKS = VER_LEGALTRADEMARKS1_STR;
const char *AxiomBackend::PRODUCT_NAME = VER_PRODUCTNAME_STR;

NumValue **AudioBackend::getAudioPortal(size_t portalId) const {
    return nullptr;
}

MidiValue **AudioBackend::getMidiPortal(size_t portalId) const {
    return nullptr;
}

const char *AudioBackend::formatNumForm(AxiomBackend::NumForm form) const {
    return "";
}

std::string AudioBackend::formatNum(AxiomBackend::NumValue value, bool includeLabel) const {
    return "";
}

QByteArray AudioBackend::serialize() {
    return QByteArray();
}

void AudioBackend::deserialize(QByteArray *data) {}

void AudioBackend::setBpm(float bpm) {}

void AudioBackend::setSampleRate(float sampleRate) {}

void AudioBackend::queueMidiEvent(size_t inSamples, size_t portalId, AxiomBackend::MidiEvent event) {}

void AudioBackend::clearMidi(size_t portalId) {}

size_t AudioBackend::beginGenerate() {
    return 1;
}

void AudioBackend::generate() {}

AudioConfiguration AudioBackend::createDefaultConfiguration() {
    return AudioConfiguration({ConfigurationPortal(PortalType::INPUT, PortalValue::MIDI, "Keyboard"),
                               ConfigurationPortal(PortalType::OUTPUT, PortalValue::AUDIO, "Speakers")});
}
