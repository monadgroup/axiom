#include "OperatorType.h"

#include <cassert>

using namespace MaximCommon;

std::string MaximCommon::operatorType2String(OperatorType type) {
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

    assert(false);
    throw;
}

std::string MaximCommon::operatorType2Verb(OperatorType type) {
    switch (type) {
        case OperatorType::IDENTITY:
            return "equate";
        case OperatorType::ADD:
            return "add";
        case OperatorType::SUBTRACT:
            return "subtract";
        case OperatorType::MULTIPLY:
            return "multiply";
        case OperatorType::DIVIDE:
            return "divide";
        case OperatorType::MODULO:
            return "modulo";
        case OperatorType::POWER:
            return "raise to a power";
        case OperatorType::BITWISE_AND:
            return "bitwise-and";
        case OperatorType::BITWISE_OR:
            return "bitwise-or";
        case OperatorType::BITWISE_XOR:
            return "bitwise-xor";
        case OperatorType::LOGICAL_AND:
            return "logical-and";
        case OperatorType::LOGICAL_OR:
            return "logical-or";
        case OperatorType::LOGICAL_EQUAL:
            return "logical-equal";
        case OperatorType::LOGICAL_NOT_EQUAL:
            return "logical-not-equal";
        case OperatorType::LOGICAL_GT:
            return "logical-greater-than";
        case OperatorType::LOGICAL_LT:
            return "logical-less-than";
        case OperatorType::LOGICAL_GTE:
            return "logical-greater-than-or-equal-to";
        case OperatorType::LOGICAL_LTE:
            return "logical-less-than-or-equal-to";
    }

    assert(false);
    throw;
}
