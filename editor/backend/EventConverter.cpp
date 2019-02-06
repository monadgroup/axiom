#include "EventConverter.h"

using namespace AxiomBackend;

std::optional<MidiEvent> AxiomBackend::convertFromMidi(int32_t event) {
    auto midiData = reinterpret_cast<uint8_t *>(&event);
    auto midiStatus = midiData[0];
    auto midiData1 = midiData[1];
    auto midiData2 = midiData[2];

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

int32_t AxiomBackend::convertToMidi(const AxiomBackend::MidiEvent &event) {
    int32_t midiEvent = 0;
    auto midiData = reinterpret_cast<uint8_t *>(&midiEvent);

    switch (event.event) {
        case MidiEventType::NOTE_ON:
            midiData[0] = 0x90;
            midiData[1] = event.note;
            midiData[2] = (uint8_t) (event.param / 2); // remap velocity from 0-255 to 0-127
            break;
        case MidiEventType::NOTE_OFF:
            midiData[0] = 0x80;
            midiData[1] = event.note;
            break;
        case MidiEventType::POLYPHONIC_AFTERTOUCH:
            midiData[0] = 0xA0;
            midiData[1] = event.note;
            midiData[2] = (uint8_t) (event.param / 2);
            break;
        case MidiEventType::CHANNEL_AFTERTOUCH:
            midiData[0] = 0xD0;
            midiData[1] = (uint8_t) (event.param / 2);
            break;
        case MidiEventType::PITCH_WHEEL:
        {
            midiData[0] = 0xE0;

            auto pitch = (uint16_t) (event.param / 255.f * 16383.f);

            // low 7 bits go in midiData[1], high 7 bits in midiData[2]
            midiData[1] = (uint8_t) (pitch & 0b1111111);
            midiData[2] = (uint8_t) ((pitch >> 7) & 0b1111111);
        }
    }

    // stuff the channel in the first half of midiData[0]
    midiData[0] |= event.channel & 0x0F;
    return midiEvent;
}
