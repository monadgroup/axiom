#pragma once

#include "../model/Value.h"

namespace AxiomBackend {
    using NumValue = AxiomModel::NumValue;
    using NumForm = AxiomModel::FormType;
    using MidiValue = AxiomModel::MidiValue;
    using MidiEvent = AxiomModel::MidiEventValue;
    using MidiEventType = AxiomModel::MidiEventType;

    extern const char *PRODUCT_VERSION;
    extern const char *COMPANY_NAME;
    extern const char *FILE_DESCRIPTION;
    extern const char *INTERNAL_NAME;
    extern const char *LEGAL_COPYRIGHT;
    extern const char *LEGAL_TRADEMARKS;
    extern const char *PRODUCT_NAME;
}
