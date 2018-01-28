#pragma once

#include <vector>
#include <memory>

#include "ParseError.h"
#include "Token.h"

namespace MaximAst {
    class Block;

    class Expression;

    class AssignableExpression;

    class Form;
}

namespace MaximParser {

    class TokenStream;

    class Parser {
    public:
        explicit Parser(std::unique_ptr<TokenStream> stream);

        TokenStream *stream() const { return _stream.get(); }

        std::unique_ptr<MaximAst::Block> parse();

        std::unique_ptr<MaximAst::Expression> parseExpression();

    private:
        // expression precedence - goes from smallest to biggest
        enum class Precedence {
            NONE = 0,

            CASTING = 1,
            UNARY = 2,

            POWER = 3,

            BITWISE = 4,

            MULTIPLY = 5,
            DIVIDE = 5,
            MODULO = 5,

            ADD = 6,
            SUBTRACT = 6,

            EQUALITY = 7,
            LOGICAL = 8,

            ASSIGNMENT = 10,

            ALL = 11
        };

        std::unique_ptr<TokenStream> _stream;

        std::unique_ptr<MaximAst::Expression> parseExpression(Precedence precedence);

        std::unique_ptr<MaximAst::Expression> parsePrefix(Precedence precedence);

        std::unique_ptr<MaximAst::Expression>
        parsePostfix(std::unique_ptr<MaximAst::Expression> prefix, Precedence precedence);

        std::unique_ptr<MaximAst::Expression> parseColonTokenExpression(Precedence precedence);

        std::unique_ptr<MaximAst::Expression> parseOpenSquareTokenExpression();

        std::unique_ptr<MaximAst::Form> parseForm();

        std::unique_ptr<MaximAst::Expression> parseNoteTokenExpression();

        std::unique_ptr<MaximAst::Expression> parseNumberTokenExpression();

        std::unique_ptr<MaximAst::Expression> parseStringTokenExpression();

        std::unique_ptr<MaximAst::Expression> parseUnaryTokenExpression();

        std::unique_ptr<MaximAst::Expression> parseIdentifierTokenExpression(Precedence precedence);

        std::unique_ptr<MaximAst::AssignableExpression> parseControlExpression(std::string name, SourcePos startPos);

        std::unique_ptr<MaximAst::Expression> parseCallExpression(std::string name, SourcePos startPos);

        std::unique_ptr<MaximAst::Expression> parseSubTokenExpression();

        void parseArguments(std::vector<std::unique_ptr<MaximAst::Expression>> &arguments);

        std::unique_ptr<MaximAst::Expression> parseCastExpression(std::unique_ptr<MaximAst::Expression> prefix);

        std::unique_ptr<MaximAst::Expression> parsePostfixExpression(std::unique_ptr<MaximAst::Expression> prefix);

        std::unique_ptr<MaximAst::Expression> parseMathExpression(std::unique_ptr<MaximAst::Expression> prefix);

        std::unique_ptr<MaximAst::Expression> parseAssignExpression(std::unique_ptr<MaximAst::Expression> prefix);

        static Precedence operatorToPrecedence(Token::Type type);

        static void pushAssignable(std::vector<std::unique_ptr<MaximAst::AssignableExpression>> &target, std::unique_ptr<MaximAst::Expression> source);

        static void expect(const Token &token, Token::Type expectedType);

        static ParseError fail(const Token &token);

        static ParseError castFail(MaximAst::Expression *expr);
    };

}
