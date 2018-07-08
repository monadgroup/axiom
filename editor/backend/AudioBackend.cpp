#include "AudioBackend.h"

using namespace AxiomBackend;

NumValue **AudioBackend::getAudioPortal(size_t portalId) const {
    return nullptr;
}

MidiValue **AudioBackend::getMidiPortal(size_t portalId) const {
    return nullptr;
}

void AudioBackend::queueMidiEvent(size_t inSamples, size_t portalId, AxiomBackend::MidiEvent event) {}

size_t AudioBackend::beginGenerate() {
    return 1;
}

void AudioBackend::generate() {}

AudioConfiguration AudioBackend::createDefaultConfiguration() {
    return AudioConfiguration({ConfigurationPortal(PortalType::INPUT, PortalValue::MIDI, "Keyboard"),
                               ConfigurationPortal(PortalType::OUTPUT, PortalValue::AUDIO, "Speakers")});
}
