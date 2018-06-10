#ifndef AXIOM_H
#define AXIOM_H

#include <stdint.h>
#include <stdbool.h>

typedef void AxiomDefinition;
typedef void AxiomInstrument;
typedef void AxiomInput;
typedef void AxiomOutput;

typedef struct {
    bool active;
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

#ifdef __cplusplus
extern "C" {
#endif
AxiomInstrument* __cdecl axiom_create_instrument(AxiomDefinition *definition);
AxiomInput* __cdecl axiom_get_input(AxiomInstrument *instrument, uint32_t id);
AxiomInput* __cdecl axiom_get_output(AxiomInstrument *instrument, uint32_t id);
void __cdecl axiom_generate();
void __cdecl axiom_destroy_instrument(AxiomInstrument *instrument);

void __cdecl axiom_midi_push(AxiomInput *input, AxiomMidiEvent event);
void __cdecl axiom_midi_clear(AxiomInput *input);
void __cdecl axiom_num_write(AxiomInput *input, AxiomNum value);
AxiomNum __cdecl axiom_num_read(AxiomOutput *output);
#ifdef __cplusplus
}
#endif

#endif
