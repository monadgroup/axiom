#include "CustomNode.h"

#include "../codegen/MaximContext.h"
#include "../parser/TokenStream.h"
#include "../parser/Parser.h"
#include "../ast/Block.h"
#include "../codegen/Value.h"
#include "Surface.h"
#include "Control.h"

using namespace MaximRuntime;

CustomNode::CustomNode(MaximCodegen::MaximContext *context, Surface *surface)
    : Node(surface), _context(context), _module("node", context->llvm()), _node(context, &_module) {

}

ErrorLog CustomNode::compile(std::string content) {
    _node.reset();

    ErrorLog log{};

    try {
        // create token stream and parser
        auto stream = std::make_unique<MaximParser::TokenStream>(content);
        MaximParser::Parser parser(std::move(stream));

        // parse and generate code
        auto block = parser.parse();
        _node.generateCode(block.get());
        _node.complete();

        // update control list
        updateControls();

        // tell the surface it needs to update
        surface()->markAsDirty();
    } catch (const MaximParser::ParseError &err) {
        log.parseErrors.push_back(err);
    } catch (const MaximCodegen::CodegenError &err) {
        log.codegenErrors.push_back(err);
    }

    return log;
}

MaximCodegen::Node* CustomNode::getFunction() {
    return &_node;
}

struct ControlUpdateVal {
    std::unique_ptr<Control> control;
    bool isUsed;

    ControlUpdateVal(std::unique_ptr<Control> control, bool isUsed) : control(std::move(control)), isUsed(isUsed) { }
};

void CustomNode::updateControls() {
    std::unordered_map<MaximCodegen::ControlKey, ControlUpdateVal> currentControls;
    for (auto &control : _controls) {
        MaximCodegen::ControlKey key { control->name(), control->type() };

        ControlUpdateVal updateVal(std::move(control), false);

        currentControls.emplace(key, std::move(updateVal));
    }
    _controls.clear();

    for (const auto &newControl : _node.controls()) {
        auto pair = currentControls.find(newControl.first);

        if (pair == currentControls.end()) {
            // it's a new control
            auto genControl = Control::create(this, newControl.first.name, newControl.second.control);

            currentControls.emplace(
                newControl.first,
                ControlUpdateVal { std::move(genControl), true }
            );
        } else {
            // it's an old control
            pair->second.isUsed = true;
            pair->second.control->setControl(newControl.second.control);
        }
    }

    for (auto &control : currentControls) {
        if (control.second.isUsed) {
            _controls.push_back(std::move(control.second.control));
        } else {
            // ensure that control is cleaned up before _any_ controls are deleted
            control.second.control->cleanup();
        }
    }
}
