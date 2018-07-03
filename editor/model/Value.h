#pragma once

#include <QtCore/QDataStream>

namespace AxiomModel {

    enum class FormType { NONE, CONTROL, OSCILLATOR, NOTE, FREQUENCY, BEATS, SECONDS, SAMPLES, DB, AMPLITUDE, Q };

    struct NumValue {
        float left = 0;
        float right = 0;
        FormType form = FormType::NONE;

        bool operator==(const NumValue &other) const {
            return left == other.left && right == other.right && form == other.form;
        }

        bool operator!=(const NumValue &other) const {
            return !(*this == other);
        }

        NumValue withLR(float l, float r) const {
            return {l, r, form};
        }

        NumValue withL(float l) const {
            return {l, right, form};
        }

        NumValue withR(float r) const {
            return {left, r, form};
        }

        NumValue withForm(FormType form) const {
            return {left, right, form};
        }
    };

    enum class MidiEventType { NOTE_ON, NOTE_OFF, POLYPHONIC_AFTERTOUCH, CHANNEL_AFTERTOUCH, PITCH_WHEEL };

    struct MidiEventValue {
        MidiEventType event = MidiEventType::NOTE_OFF;
        uint8_t channel = 0;
        uint8_t note = 0;
        uint8_t param = 0;

        bool operator==(const MidiEventValue &other) const {
            return event == other.event && channel == other.channel && note == other.note && param == other.param;
        }

        bool operator!=(const MidiEventValue &other) const {
            return !(*this == other);
        }
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

        bool operator!=(const MidiValue &other) const {
            return !(*this == other);
        }

        void pushEvent(const MidiEventValue &event) {
            if (count >= MAX_EVENTS) return;

            events[count] = event;
            count++;
        }
    };

    QDataStream &operator<<(QDataStream &stream, const NumValue &val);

    QDataStream &operator>>(QDataStream &stream, NumValue &val);

    QDataStream &operator<<(QDataStream &stream, const MidiEventValue &val);

    QDataStream &operator>>(QDataStream &stream, MidiEventValue &val);

    QDataStream &operator<<(QDataStream &stream, const MidiValue &val);

    QDataStream &operator>>(QDataStream &stream, MidiValue &val);
}
