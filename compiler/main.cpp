#include "parser/TokenStream.h"

#include <iostream>

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
    auto results = getInput();

    std::cout << "Processing: " << std::endl;
    std::cout << results << std::endl;
    std::cout << "---------" << std::endl;

    TokenStream stream(results);
    Token token(Token::Type::END_OF_FILE, "", 0, 0);
    while ((token = stream.next()).type != Token::Type::END_OF_FILE) {
        std::cout << Token::typeString(token.type);
        if (!token.content.empty()) std::cout << " \"" << token.content << "\"";
        std::cout << " (" << token.lineNumber << ":" << token.columnNumber << ")" << std::endl;
    }

    return 0;
}
