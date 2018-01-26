#pragma once

#include <string>

#include "../SourcePos.h"

namespace MaximParser {

    class Token {
    public:
        enum class Type {
            // single-char tokens
            PLUS,
            MINUS,
            TIMES,
            DIVIDE,
            MODULO,
            POWER,
            ASSIGN,
            NOT,
            OPEN_BRACKET,
            CLOSE_BRACKET,
            OPEN_SQUARE,
            CLOSE_SQUARE,
            OPEN_CURLY,
            CLOSE_CURLY,
            COMMA,
            DOT,
            COLON,
            HASH,
            GT,
            LT,
            BITWISE_AND,
            BITWISE_OR,
            END_OF_LINE,
            END_OF_FILE,

            // multi-char tokens
            GTE,
            LTE,
            EQUAL_TO,
            NOT_EQUAL_TO,
            ELLIPSIS,
            CAST,
            COMMENT_OPEN,
            COMMENT_CLOSE,
            PLUS_ASSIGN,
            MINUS_ASSIGN,
            TIMES_ASSIGN,
            DIVIDE_ASSIGN,
            MODULO_ASSIGN,
            POWER_ASSIGN,
            INCREMENT,
            DECREMENT,
            BITWISE_XOR,
            LOGICAL_AND,
            LOGICAL_OR,

            NUM_KEYWORD,
            MIDI_KEYWORD,
            PURE_KEYWORD,
            CONST_KEYWORD,

            // free tokens
            SINGLE_STRING,
            DOUBLE_STRING,
            NUMBER,
            NOTE,
            IDENTIFIER,

            UNKNOWN
        };

        Type type;
        std::string content;
        SourcePos startPos;
        SourcePos endPos;

        Token(Type type, std::string content, SourcePos startPos, SourcePos endPos);

        static std::string typeString(Type type);
    };

}
