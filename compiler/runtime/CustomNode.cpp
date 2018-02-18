#include "CustomNode.h"

#include "parser/TokenStream.h"
#include "parser/Parser.h"
#include "ast/Block.h"
#include "Runtime.h"

using namespace MaximRuntime;

CustomNode::CustomNode(Schematic *parent) : Node(parent), _node(parent->runtime()->context(), module()) {

}

void CustomNode::setCode(const std::string &code) {
    if (code != _code) {
        _code = code;
        scheduleCompile();
    }
}

void CustomNode::compile() {
    instFunc()->reset();

    _errorLog.errors.clear();

    try {
        auto stream = std::make_unique<MaximParser::TokenStream>(_code);
        MaximParser::Parser parser(std::move(stream));

        // parse and generate code
        auto block = parser.parse();
        _node.reset();
        _node.generateCode(block.get());
        _node.complete();

        instFunc()->addInstantiable(&_node);
        instFunc()->complete();
    } catch (const MaximCommon::CompileError &err) {
        _errorLog.errors.push_back(err);

        // clear flag for deploy, since we want to keep the old module loaded
        // note: this won't clear parent CompileUnit's deploy flags, so they will still deploy unnecessarily
        // would be good to fix this, but it's not too important
        cancelDeploy();
    }

    Node::compile();
}
