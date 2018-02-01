#include <iostream>

#include "parser/TokenStream.h"
#include "parser/Parser.h"
#include "ast/Block.h"

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
            block->appendString(std::cout);
        } catch (const ParseError &err) {
            std::cout << "Parse error from " << err.start.line << ":" << err.start.column << " to " << err.end.line
                      << ":" << err.end.column << std::endl;
            std::cout << err.message << std::endl;
        }
    }
}
