#include "CustomNode.h"

#include <memory>

#include "../parser/TokenStream.h"
#include "../parser/Parser.h"

using namespace MaximEditor;

ErrorLog CustomNode::updateProgram(const std::string &text) {
    ErrorLog log{};

    auto tokenStream = std::make_unique<MaximParser::TokenStream>(text);
    MaximParser::Parser parser(std::move(tokenStream));

    try {
        auto parsedBlock = parser.parse();
    } catch (const MaximParser::ParseError &error) {
        log.parseErrors.push_back(error);
    }

    return log;
}
