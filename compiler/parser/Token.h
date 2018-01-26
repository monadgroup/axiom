#pragma once

#include <string>

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
            EQUAL,
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
            END_OF_LINE,
            END_OF_FILE,

            // multi-char tokens
            GTE,
            LTE,
            EQUAL_TO,
            ELLIPSIS,
            COMMENT_OPEN,
            COMMENT_CLOSE,

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

        Token(Type type, std::string content);

        static std::string typeString(Type type);
    };

}
