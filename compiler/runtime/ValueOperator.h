#pragma once

#include "../common/FormType.h"
#include "../common/MidiEventType.h"
#include "../codegen/MidiType.h"

namespace MaximCodegen {
    class MaximContext;
}

namespace MaximRuntime {

    struct NumValue {
        float left = 0;
        float right = 0;
        MaximCommon::FormType form = MaximCommon::FormType::LINEAR;
        bool active = false;

        bool operator==(const NumValue &other) const {
            return left == other.left && right == other.right && form == other.form && active == other.active;
        }

        bool operator!=(const NumValue &other) const {
            return !(*this == other);
        }

        NumValue withLR(float l, float r) const {
            return {l, r, form, active};
        }

        NumValue withL(float l) const {
            return {l, right, form, active};
        }

        NumValue withR(float r) const {
            return {left, r, form, active};
        }
    };

    struct MidiEventValue {
        MaximCommon::MidiEventType event;
        uint8_t channel;
        uint8_t note;
        uint8_t param;

        bool operator==(const MidiEventValue &other) const {
            return event == other.event && channel == other.channel && note == other.note && param == other.param;
        }

        bool operator!=(const MidiEventValue &other) const {
            return !(*this == other);
        }
    };

    struct MidiValue {
        uint8_t count;
        MidiEventValue events[MaximCodegen::MidiType::maxEvents];

        bool operator==(const MidiValue &other) const {
            if (count != other.count) return false;
            for (uint8_t i = 0; i < count; i++) {
                if (events[i] != other.events[i]) return false;
            }
            return true;
        }

        bool operator!=(const MidiValue &other) const {
            return !(*this == other);
        }
    };

    class ValueOperator {
    public:
        explicit ValueOperator(MaximCodegen::MaximContext *context);

        NumValue readNum(void *ptr);

        void writeNum(void *ptr, const NumValue &value);

        uint8_t readMidiCount(void *ptr);

        void *getMidiEventPtr(void *ptr, uint8_t index);

        MidiEventValue readMidiEvent(void *ptr, uint8_t index);

        MidiValue readMidi(void *ptr);

        void writeMidiCount(void *ptr, uint8_t value);

        void writeMidiEvent(void *ptr, uint8_t index, const MidiEventValue &value);

        void pushMidiEvent(void *ptr, const MidiEventValue &value);

        void writeMidi(void *ptr, const MidiValue &value);

    private:
        MaximCodegen::MaximContext *_context;

        uint64_t numValOffset;
        uint64_t numFormOffset;
        uint64_t numActiveOffset;

        uint64_t midiCountOffset;
        uint64_t midiArrayOffset;
        uint64_t midiEventSize;
        uint64_t midiEventTypeOffset;
        uint64_t midiEventChannelOffset;
        uint64_t midiEventNoteOffset;
        uint64_t midiEventParamOffset;
    };

}
