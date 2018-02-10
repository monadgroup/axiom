#pragma once

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

    std::string operatorType2String(OperatorType type) {
        switch (type) {
            case OperatorType::IDENTITY:
                return "=";
            case OperatorType::ADD:
                return "+";
            case OperatorType::SUBTRACT:
                return "-";
            case OperatorType::MULTIPLY:
                return "*";
            case OperatorType::DIVIDE:
                return "/";
            case OperatorType::MODULO:
                return "%";
            case OperatorType::POWER:
                return "^";
            case OperatorType::BITWISE_AND:
                return "&";
            case OperatorType::BITWISE_OR:
                return "|";
            case OperatorType::BITWISE_XOR:
                return "^^";
            case OperatorType::LOGICAL_AND:
                return "&&";
            case OperatorType::LOGICAL_OR:
                return "||";
            case OperatorType::LOGICAL_EQUAL:
                return "==";
            case OperatorType::LOGICAL_NOT_EQUAL:
                return "!=";
            case OperatorType::LOGICAL_GT:
                return ">";
            case OperatorType::LOGICAL_LT:
                return "<";
            case OperatorType::LOGICAL_GTE:
                return ">=";
            case OperatorType::LOGICAL_LTE:
                return "<=";
        }
    }

}
