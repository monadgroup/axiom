#pragma once

#include <istream>
#include <regex>

#include "Token.h"

namespace MaximParser {

    class TokenStream {
    public:

        explicit TokenStream(const std::string &data);

        void restart();

        Token next();

        Token peek();

    private:
        std::string data;
        std::string::iterator dataBegin;
        int currentLine = 0;
        int currentColumn = 0;

        bool hasBuffer = false;
        Token peekBuffer;

        Token processNext();

        bool isSingleLineComment = false;
        int multiLineCommentCount = 0;

        bool filter(const Token &token);

        static constexpr std::size_t matchCount = 51;

        using PairType = std::pair<std::regex, Token::Type>;
        using PairListType = std::array<PairType, matchCount>;

        static PairListType matches;
        static PairType getToken(const std::string &regex, Token::Type type);
    };

}
