#pragma once

#include "../Value.h"
#include <QDataStream>

namespace AxiomModel {

    namespace ValueSerializer {
        void serializeNum(const NumValue &val, QDataStream &stream);

        NumValue deserializeNum(QDataStream &stream, uint32_t version);

        void serializeMidiEvent(const MidiEventValue &val, QDataStream &stream);

        MidiEventValue deserializeMidiEvent(QDataStream &stream, uint32_t version);

        void serializeMidi(const MidiValue &val, QDataStream &stream);

        MidiValue deserializeMidi(QDataStream &stream, uint32_t version);
    }
}
