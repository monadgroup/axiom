#include "EventConverter.h"

using namespace AxiomBackend;

std::optional<MidiEvent> AxiomBackend::convertFromVst(VstMidiEvent *event) {
    auto midiStatus = event->midiData[0];
    auto midiData1 = event->midiData[1];
    auto midiData2 = event->midiData[2];

    auto eventType = (uint8_t) (midiStatus & 0xF0);
    auto eventChannel = (uint8_t) (midiStatus & 0x0F);

    MidiEvent remappedEvent;
    remappedEvent.channel = eventChannel;

    switch (eventType) {
    case 0x80: // note off
        remappedEvent.event = MidiEventType::NOTE_OFF;
        remappedEvent.note = (uint8_t) midiData1;
        return remappedEvent;
    case 0x90: // note on
        remappedEvent.event = MidiEventType::NOTE_ON;
        remappedEvent.note = (uint8_t) midiData1;
        remappedEvent.param = (uint8_t) (midiData2 * 2); // MIDI velocity is 0-127, we need 0-255
        return remappedEvent;
    case 0xA0:
        remappedEvent.event = MidiEventType::POLYPHONIC_AFTERTOUCH;
        remappedEvent.note = (uint8_t) midiData1;
        remappedEvent.param = (uint8_t) (midiData2 * 2); // MIDI aftertouch pressure is 0-127, we need 0-255
        return remappedEvent;
    case 0xD0:
        remappedEvent.event = MidiEventType::CHANNEL_AFTERTOUCH;
        remappedEvent.param = (uint8_t) (midiData1 * 2); // MIDI aftertouch pressure is 0-127, we need 0-255
        return remappedEvent;
    case 0xE0: // pitch wheel
    {
        remappedEvent.event = MidiEventType::PITCH_WHEEL;

        // Pitch is 0-0x3FFF stored across the two bytes, we need 0-255
        // todo: we should allow a larger range in the internal events
        auto pitch = ((uint16_t) midiData2 << 7) | (uint16_t) midiData1;
        remappedEvent.param = (uint8_t) (pitch / 16383.f * 255.f);
        return remappedEvent;
    }
    default:
        return std::nullopt;
    }
}
