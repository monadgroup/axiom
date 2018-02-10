#pragma once

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

    std::string formType2String(FormType type) {
        switch (type) {
            case FormType::LINEAR: return "lin";
            case FormType::CONTROL: return "control";
            case FormType::FREQUENCY: return "freq";
            case FormType::NOTE: return "note";
            case FormType::DB: return "db";
            case FormType::Q: return "q";
            case FormType::RESONANCE: return "res";
            case FormType::SECONDS: return "secs";
            case FormType::BEATS: return "beats";
        }
    }

}
