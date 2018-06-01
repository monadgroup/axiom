#include "FormType.h"

#include <cassert>

using namespace MaximCommon;

std::string MaximCommon::formType2String(FormType type) {
    switch (type) {
        case FormType::NONE:
            return "none";
        case FormType::CONTROL:
            return "control";
        case FormType::OSCILLATOR:
            return "osc";
        case FormType::NOTE:
            return "note";
        case FormType::FREQUENCY:
            return "freq";
        case FormType::BEATS:
            return "beats";
        case FormType::SECONDS:
            return "secs";
        case FormType::SAMPLES:
            return "samples";
        case FormType::DB:
            return "db";
        case FormType::AMPLITUDE:
            return "amp";
        case FormType::Q:
            return "q";
    }

    assert(false);
    throw;
}
