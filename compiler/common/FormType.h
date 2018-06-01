#pragma once

#include <string>

namespace MaximCommon {

    enum class FormType {
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

    std::string formType2String(FormType type);

}
