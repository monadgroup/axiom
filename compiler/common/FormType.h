#pragma once

#include <string>

namespace MaximCommon {

    enum class FormType {
        LINEAR,
        CONTROL,
        FREQUENCY,
        NOTE,
        DB,
        Q,
        RESONANCE,
        SECONDS,
        BEATS
    };

    std::string formType2String(FormType type);

}
