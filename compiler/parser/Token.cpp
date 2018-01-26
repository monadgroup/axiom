#include "Token.h"

using namespace MaximParser;

Token::Token(Type type, std::string content) : type(type), content(std::move(content)) {

}

std::string Token::typeString(Type type) {
    switch (type) {
        case Type::PLUS: return "+";
        case Type::MINUS: return "-";
        case Type::TIMES: return "*";
        case Type::DIVIDE: return "/";
        case Type::MODULO: return "%";
        case Type::POWER: return "^";
        case Type::EQUAL: return "=";
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
        case Type::END_OF_LINE: return "EOL";
        case Type::END_OF_FILE: return "EOF";
        case Type::GTE: return ">=";
        case Type::LTE: return "<=";
        case Type::EQUAL_TO: return "==";
        case Type::ELLIPSIS: return "...";
        case Type::COMMENT_OPEN: return "/*";
        case Type::COMMENT_CLOSE: return "*/";
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
