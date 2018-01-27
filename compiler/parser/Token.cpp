#include "Token.h"

using namespace MaximParser;

Token::Token(Type type, std::string content, SourcePos startPos, SourcePos endPos)
        : type(type), content(std::move(content)), startPos(startPos), endPos(endPos) {

}

std::string Token::typeString(Type type) {
    switch (type) {
        case Type::PLUS: return "addition operator";
        case Type::MINUS: return "subtraction operator";
        case Type::TIMES: return "multiplication operator";
        case Type::DIVIDE: return "division operator";
        case Type::MODULO: return "modulo operator";
        case Type::POWER: return "power operator";
        case Type::ASSIGN: return "assignment operator";
        case Type::NOT: return "NOT operator";
        case Type::OPEN_BRACKET: return "open bracket";
        case Type::CLOSE_BRACKET: return "close bracket";
        case Type::OPEN_SQUARE: return "open square bracket";
        case Type::CLOSE_SQUARE: return "close square bracket";
        case Type::OPEN_CURLY: return "open curly bracket";
        case Type::CLOSE_CURLY: return "close curly bracket";
        case Type::COMMA: return "comma";
        case Type::SEMICOLON: return "semicolon";
        case Type::DOT: return "dot";
        case Type::COLON: return "colon";
        case Type::HASH: return "comment";
        case Type::GT: return "greater-than operator";
        case Type::LT: return "less-than operator";
        case Type::BITWISE_AND: return "bitwise AND operator";
        case Type::BITWISE_OR: return "bitwise OR operator";
        case Type::END_OF_LINE: return "newline";
        case Type::END_OF_FILE: return "end-of-file";
        case Type::GTE: return "greater-than-or-equal-to operator";
        case Type::LTE: return "less-than-or-equal-to operator";
        case Type::EQUAL_TO: return "equal-to operator";
        case Type::NOT_EQUAL_TO: return "not-equal-to operator";
        case Type::ELLIPSIS: return "spread operator";
        case Type::CAST: return "arrow";
        case Type::COMMENT_OPEN: return "comment open";
        case Type::COMMENT_CLOSE: return "comment close";
        case Type::PLUS_ASSIGN: return "add-to operator";
        case Type::MINUS_ASSIGN: return "subtract-from operator";
        case Type::TIMES_ASSIGN: return "multiply-by operator";
        case Type::DIVIDE_ASSIGN: return "divide-by operator";
        case Type::MODULO_ASSIGN: return "modulo-by operator";
        case Type::POWER_ASSIGN: return "raise-to operator";
        case Type::INCREMENT: return "increment operator";
        case Type::DECREMENT: return "decrement operator";
        case Type::BITWISE_XOR: return "bitwise XOR operator";
        case Type::LOGICAL_AND: return "logical AND operator";
        case Type::LOGICAL_OR: return "logical OR operator";
        case Type::NUM_KEYWORD: return "NUM keyword";
        case Type::MIDI_KEYWORD: return "MIDI keyword";
        case Type::PURE_KEYWORD: return "PURE keyword";
        case Type::CONST_KEYWORD: return "CONST keyword";
        case Type::SINGLE_STRING: return "string";
        case Type::DOUBLE_STRING: return "string";
        case Type::NUMBER: return "number";
        case Type::NOTE: return "note";
        case Type::IDENTIFIER: return "identifier";
        case Type::UNKNOWN: return "...well, I'm not really sure what this is...";
        default: return " -- 42 --";
    }
}
