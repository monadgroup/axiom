#pragma once

#include <string>

namespace MaximCommon {

    enum class OperatorType {
        IDENTITY,

        ADD,
        SUBTRACT,
        MULTIPLY,
        DIVIDE,
        MODULO,
        POWER,

        BITWISE_AND,
        BITWISE_OR,
        BITWISE_XOR,
        LOGICAL_AND,
        LOGICAL_OR,
        LOGICAL_EQUAL,
        LOGICAL_NOT_EQUAL,
        LOGICAL_GT,
        LOGICAL_LT,
        LOGICAL_GTE,
        LOGICAL_LTE
    };

    std::string operatorType2String(OperatorType type);

    std::string operatorType2Verb(OperatorType type);

}
