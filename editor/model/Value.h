#pragma once

#include <QtCore/QDataStream>
#include <emmintrin.h>

namespace AxiomModel {

    // NOTE: all structs here must match those defined in the compiler.

    struct ArrayValue {
        uint32_t flags;
    };

    enum class FormType : uint8_t {
        NONE,
        CONTROL,
        OSCILLATOR,
        NOTE,
        FREQUENCY,
        BEATS,
        SECONDS,
        SAMPLES,
        DB,
        AMPLITUDE,
        Q
    };

    struct NumValue {
        union {
            struct {
                double left;
                double right;
            };
            __m128 value;
        };
        FormType form;

        NumValue() : left(0), right(0), form(FormType::NONE) {}
        NumValue(double left, double right, FormType form) : left(left), right(right), form(form) {}

        bool operator==(const NumValue &other) const {
            return left == other.left && right == other.right && form == other.form;
        }

        bool operator!=(const NumValue &other) const { return !(*this == other); }

        NumValue withLR(double l, double r) const { return {l, r, form}; }

        NumValue withL(double l) const { return {l, right, form}; }

        NumValue withR(double r) const { return {left, r, form}; }

        NumValue withForm(FormType form) const { return {left, right, form}; }
    };

    enum class MidiEventType : uint8_t { NOTE_ON, NOTE_OFF, POLYPHONIC_AFTERTOUCH, CHANNEL_AFTERTOUCH, PITCH_WHEEL };

    struct MidiEventValue {
        MidiEventType event = MidiEventType::NOTE_OFF;
        uint8_t channel = 0;
        uint8_t note = 0;
        uint8_t param = 0;

        bool operator==(const MidiEventValue &other) const {
            return event == other.event && channel == other.channel && note == other.note && param == other.param;
        }

        bool operator!=(const MidiEventValue &other) const { return !(*this == other); }
    };

    struct MidiValue {
        static constexpr size_t MAX_EVENTS = 32;

        uint8_t count = 0;
        MidiEventValue events[MAX_EVENTS];

        bool operator==(const MidiValue &other) const {
            if (count != other.count) return false;
            for (uint8_t i = 0; i < count; i++) {
                if (events[i] != other.events[i]) return false;
            }
            return true;
        }

        bool operator!=(const MidiValue &other) const { return !(*this == other); }

        void pushEvent(const MidiEventValue &event) {
            if (count >= MAX_EVENTS) return;

            events[count] = event;
            count++;
        }
    };
}
