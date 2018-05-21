#include "ValueOperator.h"

#include "../codegen/MaximContext.h"

using namespace MaximRuntime;

ValueOperator::ValueOperator(MaximCodegen::MaximContext *context) : _context(context) {
    auto numLayout = context->numType()->layout();
    numActiveOffset = numLayout->getElementOffset(0);
    numValOffset = numLayout->getElementOffset(1);
    numFormOffset = numLayout->getElementOffset(2);
    numStride = numLayout->getSizeInBytes();

    auto midiLayout = context->midiType()->layout();
    midiActiveOffset = midiLayout->getElementOffset(0);
    midiCountOffset = midiLayout->getElementOffset(1);
    midiArrayOffset = midiLayout->getElementOffset(2);
    midiStride = midiLayout->getSizeInBytes();

    auto midiEventLayout = context->midiType()->eventLayout();
    midiEventSize = midiEventLayout->getSizeInBytes();
    midiEventTypeOffset = midiEventLayout->getElementOffset(0);
    midiEventChannelOffset = midiEventLayout->getElementOffset(1);
    midiEventNoteOffset = midiEventLayout->getElementOffset(2);
    midiEventParamOffset = midiEventLayout->getElementOffset(3);
}

uint32_t ValueOperator::readArrayActiveFlags(void *ptr, size_t stride, size_t offset) {
    auto bytePtr = (uint8_t *) ptr;
    uint32_t result = 0;
    for (size_t i = 0; i < MaximCodegen::ArrayType::arraySize; i++) {
        auto isActive = (bool) *(bytePtr + i * stride + offset);
        result |= isActive << i;
    }
    return result;
}

uint32_t ValueOperator::readNumArrayActiveFlags(void *ptr) {
    return readArrayActiveFlags(ptr, numStride, numActiveOffset);
}

uint32_t ValueOperator::readMidiArrayActiveFlags(void *ptr) {
    return readArrayActiveFlags(ptr, midiStride, midiActiveOffset);
}

NumValue ValueOperator::readNum(void *ptr) {
    auto bytePtr = (uint8_t *) ptr;
    auto vecPtr = (float *) (bytePtr + numValOffset);
    return {
        (bool) *(bytePtr + numActiveOffset),
        *(vecPtr + 0),
        *(vecPtr + 1),
        (MaximCommon::FormType) *(bytePtr + numFormOffset)
    };
}

void ValueOperator::writeNum(void *ptr, const NumValue &value) {
    auto bytePtr = (uint8_t *) ptr;
    auto vecPtr = (float *) (bytePtr + numValOffset);
    *(vecPtr + 0) = value.left;
    *(vecPtr + 1) = value.right;
    *(bytePtr + numFormOffset) = (uint8_t) value.form;
    *(bytePtr + numActiveOffset) = (uint8_t) value.active;
}

bool ValueOperator::readMidiActive(void *ptr) {
    auto bytePtr = (uint8_t *) ptr;
    return *(bytePtr + midiActiveOffset);
}

uint8_t ValueOperator::readMidiCount(void *ptr) {
    auto bytePtr = (uint8_t *) ptr;
    return *(bytePtr + midiCountOffset);
}

void *ValueOperator::getMidiEventPtr(void *ptr, uint8_t index) {
    auto arrayPtr = ((uint8_t *) ptr) + midiArrayOffset;
    return arrayPtr + midiEventSize * index;
}

MidiEventValue ValueOperator::readMidiEvent(void *ptr, uint8_t index) {
    auto eventPtr = (uint8_t *) getMidiEventPtr(ptr, index);

    return {
        (MaximCommon::MidiEventType) *(eventPtr + midiEventTypeOffset),
        *(eventPtr + midiEventChannelOffset),
        *(eventPtr + midiEventNoteOffset),
        *(eventPtr + midiEventParamOffset)
    };
}

MidiValue ValueOperator::readMidi(void *ptr) {
    MidiValue result;
    result.active = readMidiActive(ptr);
    result.count = readMidiCount(ptr);

    for (uint8_t i = 0; i < result.count; i++) {
        result.events[i] = readMidiEvent(ptr, i);
    }

    return result;
}

void ValueOperator::writeMidiActive(void *ptr, bool active) {
    auto bytePtr = (uint8_t *) ptr;
    *(bytePtr + midiActiveOffset) = (uint8_t) active;
}

void ValueOperator::writeMidiCount(void *ptr, uint8_t value) {
    auto bytePtr = (uint8_t *) ptr;
    *(bytePtr + midiCountOffset) = value;
}

void ValueOperator::writeMidiEvent(void *ptr, uint8_t index, const MidiEventValue &value) {
    auto eventPtr = (uint8_t *) getMidiEventPtr(ptr, index);

    *(eventPtr + midiEventTypeOffset) = (uint8_t) value.event;
    *(eventPtr + midiEventChannelOffset) = value.channel;
    *(eventPtr + midiEventNoteOffset) = value.note;
    *(eventPtr + midiEventParamOffset) = value.param;
}

void ValueOperator::pushMidiEvent(void *ptr, const MidiEventValue &value) {
    auto count = readMidiCount(ptr);
    if (count >= MaximCodegen::MidiType::maxEvents) return;

    writeMidiEvent(ptr, count, value);
    writeMidiCount(ptr, (uint8_t) (count + 1));
}

void ValueOperator::writeMidi(void *ptr, const MidiValue &value) {
    writeMidiActive(ptr, value.active);
    writeMidiCount(ptr, value.count);
    for (uint8_t i = 0; i < value.count; i++) {
        writeMidiEvent(ptr, i, value.events[i]);
    }
}
