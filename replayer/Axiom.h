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

typedef struct {
    uint8_t type;
    uint8_t channel;
    uint8_t note;
    uint8_t param;
} AxiomMidiEvent;

AxiomInstrument *axiom_create_instrument(AxiomDefinition *definition);
AxiomInput *axiom_get_input(AxiomInstrument *instrument, uint8_t id);
AxiomInput *axiom_get_output(AxiomInstrument *instrument, uint8_t id);
void axiom_generate();
void axiom_destroy_instrument(AxiomInstrument *instrument);

void axiom_midi_push(AxiomInput *input, AxiomMidiEvent event);
void axiom_midi_clear(AxiomInput *input);
void axiom_num_write(AxiomInput *input, AxiomNum value);
AxiomNum axiom_num_read(AxiomOutput *output);

#endif
