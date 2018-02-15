#pragma once

#include <string>

namespace MaximCommon {

    enum class FormType {
        LINEAR,
        OSCILLATOR,
        CONTROL,
        NOTE,
        DB,
        Q,
        SECONDS,
        BEATS
    };

    std::string formType2String(FormType type);

}
