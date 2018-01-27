#include <iostream>
#include "TokenStream.h"

using namespace MaximParser;

TokenStream::TokenStream(const std::string &data)
        : data(data), peekBuffer(Token(Token::Type::END_OF_FILE, "", SourcePos(0, 0), SourcePos(0, 0))) {
    restart();
}

void TokenStream::restart() {
    dataBegin = data.begin();
    hasBuffer = false;
    currentLine = 0;
    currentColumn = 0;
}

Token TokenStream::next() {
    if (hasBuffer) {
        hasBuffer = false;
        return peekBuffer;
    }
    auto nextToken = processNext();
    return nextToken;
}

Token TokenStream::peek() {
    if (!hasBuffer) {
        hasBuffer = true;
        peekBuffer = processNext();
    }

    auto nextToken = peekBuffer;

    return peekBuffer;
}

Token TokenStream::processNext() {
    if (dataBegin >= data.end()) {
        return Token(
                Token::Type::END_OF_FILE, "",
                SourcePos(currentLine, currentColumn),
                SourcePos(currentLine, currentColumn)
        );
    }

    bool hasToken = false;
    Token pickToken(Token::Type::UNKNOWN, "", SourcePos(0, 0), SourcePos(0, 0));

    do {
        for (const auto &token : TokenStream::matches) {
            std::match_results<std::string::iterator> m;
            if (!std::regex_search(
                    dataBegin, data.end(),
                    m, token.first,
                    std::regex_constants::match_continuous
            ))
                continue;

            dataBegin += m.length();

            auto tokenLine = currentLine;
            auto tokenColumn = currentColumn;

            if (token.second == Token::Type::END_OF_LINE) {
                currentLine++;
                currentColumn = 0;
            } else {
                currentColumn += m.length();
            }

            auto tokenContent = m.size() > 1 ? m[1].str() : "";
            hasToken = true;
            pickToken = Token(
                    token.second, tokenContent,
                    SourcePos(tokenLine, tokenColumn),
                    SourcePos(tokenLine, tokenColumn + m.length())
            );
            break;
        }
    } while (!filter(pickToken));

    if (hasToken) return pickToken;

    auto remainingStr = std::string(dataBegin, data.end());
    dataBegin = data.end();
    return Token(
            Token::Type::UNKNOWN, remainingStr,
            SourcePos(currentLine, currentColumn),
            SourcePos(currentLine, currentColumn)
    );
}

bool TokenStream::filter(const Token &token) {
    if (token.type == Token::Type::HASH) {
        isSingleLineComment = true;
    } else if (token.type == Token::Type::END_OF_LINE) {
        isSingleLineComment = false;
    } else if (token.type == Token::Type::COMMENT_OPEN) {
        multiLineCommentCount++;
    }

    auto isValid = !isSingleLineComment && !multiLineCommentCount;

    if (token.type == Token::Type::COMMENT_CLOSE) {
        multiLineCommentCount--;
        if (multiLineCommentCount < 0) multiLineCommentCount = 0;
    }

    return isValid;
}

TokenStream::PairListType TokenStream::matches = {
        // multi-char tokens
        getToken(R"(\.\.\.)", Token::Type::ELLIPSIS),
        getToken(R"(==)", Token::Type::EQUAL_TO),
        getToken(R"(!=)", Token::Type::NOT_EQUAL_TO),
        getToken(R"(<=)", Token::Type::LTE),
        getToken(R"(>=)", Token::Type::GTE),
        getToken(R"(\+=)", Token::Type::PLUS_ASSIGN),
        getToken(R"(-=)", Token::Type::MINUS_ASSIGN),
        getToken(R"(\*=)", Token::Type::TIMES_ASSIGN),
        getToken(R"(\/=)", Token::Type::DIVIDE_ASSIGN),
        getToken(R"(%=)", Token::Type::MODULO_ASSIGN),
        getToken(R"(\^=)", Token::Type::POWER_ASSIGN),
        getToken(R"(->)", Token::Type::CAST),
        getToken(R"(\+\+)", Token::Type::INCREMENT),
        getToken(R"(--)", Token::Type::DECREMENT),
        getToken(R"(\^\^)", Token::Type::BITWISE_XOR),
        getToken(R"(&&)", Token::Type::LOGICAL_AND),
        getToken(R"(\|\|)", Token::Type::LOGICAL_OR),
        getToken(R"(\/\*)", Token::Type::COMMENT_OPEN),
        getToken(R"(\*\/)", Token::Type::COMMENT_CLOSE),

        // multi-char identifiers
        getToken(R"(\bnum\b)", Token::Type::NUM_KEYWORD),
        getToken(R"(\bmidi\b)", Token::Type::MIDI_KEYWORD),
        getToken(R"(\bpure\b)", Token::Type::PURE_KEYWORD),
        getToken(R"(\bconst\b)", Token::Type::CONST_KEYWORD),

        // free tokens
        getToken(R"('((?:\\'|(?:(?!').))*)')", Token::Type::SINGLE_STRING),
        getToken(R"(\"((?:\\\"|(?:(?!\").))*)\")", Token::Type::DOUBLE_STRING),
        getToken(R"(((?=\.\d|\d)(?:\d+)?(?:\.?\d*)(?:[eE][+-]?\d+)?))", Token::Type::NUMBER),
        getToken(R"(:([a-gA-G]#?[0-9]+))", Token::Type::NOTE),
        getToken(R"(([_a-zA-Z][_a-zA-Z0-9]*))", Token::Type::IDENTIFIER),

        // single-char tokens
        getToken(R"(\+)", Token::Type::PLUS),
        getToken(R"(-)", Token::Type::MINUS),
        getToken(R"(\*)", Token::Type::TIMES),
        getToken(R"(\/)", Token::Type::DIVIDE),
        getToken(R"(%)", Token::Type::MODULO),
        getToken(R"(\^)", Token::Type::POWER),
        getToken(R"(=)", Token::Type::ASSIGN),
        getToken(R"(!)", Token::Type::NOT),
        getToken(R"(\()", Token::Type::OPEN_BRACKET),
        getToken(R"(\))", Token::Type::CLOSE_BRACKET),
        getToken(R"(\[)", Token::Type::OPEN_SQUARE),
        getToken(R"(\])", Token::Type::CLOSE_SQUARE),
        getToken(R"(\{)", Token::Type::OPEN_CURLY),
        getToken(R"(\})", Token::Type::CLOSE_CURLY),
        getToken(R"(,)", Token::Type::COMMA),
        getToken(R"(;)", Token::Type::SEMICOLON),
        getToken(R"(\.)", Token::Type::DOT),
        getToken(R"(:)", Token::Type::COLON),
        getToken(R"(#)", Token::Type::HASH),
        getToken(R"(>)", Token::Type::GT),
        getToken(R"(<)", Token::Type::LT),
        getToken(R"(&)", Token::Type::BITWISE_AND),
        getToken(R"(\|)", Token::Type::BITWISE_OR),

        getToken(R"(\n)", Token::Type::END_OF_LINE)
};

TokenStream::PairType TokenStream::getToken(const std::string &regex, Token::Type type) {
    return std::make_pair(
            std::regex(R"([^\S\n]*)" + regex + R"([^\S\n]*)", std::regex::ECMAScript | std::regex::optimize),
            type
    );
}
