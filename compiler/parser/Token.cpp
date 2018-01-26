#include "Token.h"

using namespace MaximParser;

Token::Token(Type type, std::string content, SourcePos startPos, SourcePos endPos)
        : type(type), content(std::move(content)), startPos(startPos), endPos(endPos) {

}

std::string Token::typeString(Type type) {
    switch (type) {
        case Type::PLUS: return "+";
        case Type::MINUS: return "-";
        case Type::TIMES: return "*";
        case Type::DIVIDE: return "/";
        case Type::MODULO: return "%";
        case Type::POWER: return "^";
        case Type::ASSIGN: return "=";
        case Type::NOT: return "!";
        case Type::OPEN_BRACKET: return "(";
        case Type::CLOSE_BRACKET: return ")";
        case Type::OPEN_SQUARE: return "[";
        case Type::CLOSE_SQUARE: return "]";
        case Type::OPEN_CURLY: return "{";
        case Type::CLOSE_CURLY: return "}";
        case Type::COMMA: return ",";
        case Type::DOT: return ".";
        case Type::COLON: return ":";
        case Type::HASH: return "#";
        case Type::GT: return ">";
        case Type::LT: return "<";
        case Type::BITWISE_AND: return "&";
        case Type::BITWISE_OR: return "|";
        case Type::END_OF_LINE: return "EOL";
        case Type::END_OF_FILE: return "EOF";
        case Type::GTE: return ">=";
        case Type::LTE: return "<=";
        case Type::EQUAL_TO: return "==";
        case Type::NOT_EQUAL_TO: return "!=";
        case Type::ELLIPSIS: return "...";
        case Type::CAST: return "->";
        case Type::COMMENT_OPEN: return "/*";
        case Type::COMMENT_CLOSE: return "*/";
        case Type::PLUS_ASSIGN: return "+=";
        case Type::MINUS_ASSIGN: return "-=";
        case Type::TIMES_ASSIGN: return "*=";
        case Type::DIVIDE_ASSIGN: return "/=";
        case Type::MODULO_ASSIGN: return "%=";
        case Type::POWER_ASSIGN: return "^=";
        case Type::INCREMENT: return "++";
        case Type::DECREMENT: return "--";
        case Type::BITWISE_XOR: return "^^";
        case Type::LOGICAL_AND: return "&&";
        case Type::LOGICAL_OR: return "||";
        case Type::NUM_KEYWORD: return "NUM";
        case Type::MIDI_KEYWORD: return "MIDI";
        case Type::PURE_KEYWORD: return "PURE";
        case Type::CONST_KEYWORD: return "CONST";
        case Type::SINGLE_STRING: return "single string";
        case Type::DOUBLE_STRING: return "double string";
        case Type::NUMBER: return "number";
        case Type::NOTE: return "note";
        case Type::IDENTIFIER: return "id";
        case Type::UNKNOWN: return "??";
    }
}
