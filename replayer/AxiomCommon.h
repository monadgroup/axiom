#ifndef AXIOM_COMMON_H
#define AXIOM_COMMON_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    float left;
    float right;
    uint8_t form;
} AxiomNum;

typedef enum : uint8_t {
    AXIOM_EVENT_NOTE_ON,
    AXIOM_EVENT_NOTE_OFF,
    AXIOM_EVENT_POLYPHONIC_AFTERTOUCH,
    AXIOM_EVENT_CHANNEL_AFTERTOUCH,
    AXIOM_EVENT_PITCH_WHEEL
} AxiomMidiEventType;

typedef struct {
    AxiomMidiEventType type;
    uint8_t channel;
    uint8_t note;
    uint8_t param;
} AxiomMidiEvent;

typedef struct {
    uint8_t event_count;
    AxiomMidiEvent events[16];
} AxiomMidi;

#endif
