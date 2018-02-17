#include "CustomNode.h"

#include <llvm/Transforms/Utils/Cloning.h>

#include "../codegen/MaximContext.h"
#include "../parser/TokenStream.h"
#include "../parser/Parser.h"
#include "../ast/Block.h"
#include "../codegen/Value.h"
#include "Surface.h"
#include "Control.h"

using namespace MaximRuntime;

CustomNode::CustomNode(Surface *surface)
    : Node(surface), _surface(surface), _module("node", surface->context()->llvm()), _node(surface->context(), &_module) {
    _module.setDataLayout(surface->context()->dataLayout());
}

CustomNode::~CustomNode() {
    if (_hasHandle) _surface->runtime()->removeModule(_handle);
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

        if (_hasHandle) _surface->runtime()->removeModule(_handle);
        _handle = _surface->runtime()->addModule(llvm::CloneModule(_module));
        _hasHandle = true;
    } catch (const MaximCommon::CompileError &err) {
        log.errors.push_back(err);
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
