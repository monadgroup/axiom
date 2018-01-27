#include <iostream>

#include "parser/TokenStream.h"
#include "parser/Parser.h"
#include "ast/Block.h"
#include "ast/Expression.h"
#include "ast/UnaryExpression.h"
#include "ast/VariableExpression.h"
#include "ast/ControlExpression.h"
#include "ast/CallExpression.h"
#include "ast/CastExpression.h"
#include "ast/NoteExpression.h"
#include "ast/NumberExpression.h"
#include "ast/PostfixExpression.h"
#include "ast/MathExpression.h"
#include "ast/AssignExpression.h"

using namespace MaximParser;

std::string getInput() {
    std::cin >> std::noskipws;

    std::string accumulate;
    std::string lastLine;

    while (std::getline(std::cin, lastLine)) {
        if (lastLine == "end") break;
        accumulate += lastLine + "\n";
    }
    return accumulate;
}

int main() {
    while (true) {
        auto results = getInput();

        auto stream = std::make_unique<TokenStream>(results);
        Parser parser(std::move(stream));

        try {
            auto block = parser.parse();
            std::cout << block->toString() << std::endl;
        } catch (const ParseError &err) {
            std::cout << "Parse error from " << err.start.line << ":" << err.start.column << " to " << err.end.line
                      << ":" << err.end.column << std::endl;
            std::cout << err.message << std::endl;
        }
    }
}
