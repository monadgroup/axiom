#include "ValueOperator.h"

#include "../codegen/MaximContext.h"

using namespace MaximRuntime;

ValueOperator::ValueOperator(MaximCodegen::MaximContext *context) : _context(context) {
    auto numLayout = context->numType()->layout();
    numValOffset = numLayout->getElementOffset(0);
    numFormOffset = numLayout->getElementOffset(1);
    numActiveOffset = numLayout->getElementOffset(2);

    auto midiLayout = context->midiType()->layout();
    midiCountOffset = midiLayout->getElementOffset(0);
    midiArrayOffset = midiLayout->getElementOffset(1);

    auto midiEventLayout = context->midiType()->eventLayout();
    midiEventSize = midiEventLayout->getSizeInBytes();
    midiEventTypeOffset = midiEventLayout->getElementOffset(0);
    midiEventChannelOffset = midiEventLayout->getElementOffset(1);
    midiEventNoteOffset = midiEventLayout->getElementOffset(2);
    midiEventParamOffset = midiEventLayout->getElementOffset(3);
}

NumValue ValueOperator::readNum(void *ptr) {
    auto bytePtr = (uint8_t *) ptr;
    auto vecPtr = (float *) (bytePtr + numValOffset);
    return {
        *(vecPtr + 0),
        *(vecPtr + 1),
        (MaximCommon::FormType) *(bytePtr + numFormOffset),
        (bool) *(bytePtr + numActiveOffset)
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

uint8_t ValueOperator::readMidiCount(void *ptr) {
    auto bytePtr = (uint8_t *) ptr;
    return *(bytePtr + midiCountOffset);
}

void* ValueOperator::getMidiEventPtr(void *ptr, uint8_t index) {
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
    result.count = readMidiCount(ptr);

    for (uint8_t i = 0; i < result.count; i++) {
        result.events[i] = readMidiEvent(ptr, i);
    }

    return result;
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
    writeMidiCount(ptr, (uint8_t)(count + 1));
}

void ValueOperator::writeMidi(void *ptr, const MidiValue &value) {
    writeMidiCount(ptr, value.count);
    for (uint8_t i = 0; i < value.count; i++) {
        writeMidiEvent(ptr, i, value.events[i]);
    }
}
