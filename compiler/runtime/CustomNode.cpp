#include <iostream>
#include "CustomNode.h"

#include "Surface.h"
#include "Runtime.h"
#include "GeneratableModuleClass.h"
#include "../parser/TokenStream.h"
#include "../parser/Parser.h"
#include "../ast/Block.h"
#include "../codegen/visitors/ExpressionVisitor.h"
#include "../codegen/Control.h"
#include "../codegen/Value.h"

using namespace MaximRuntime;

CustomNode::CustomNode(Surface *surface) : Node(surface) {
    setCode("");
}

CustomNode::~CustomNode() = default;

void CustomNode::setCode(const std::string &code) {
    if (code != _code) {
        _code = code;

        try {
            auto stream = std::make_unique<MaximParser::TokenStream>(_code);
            MaximParser::Parser parser(std::move(stream));
            _ast = std::move(parser.parse());

            scheduleCompile();
        } catch (const MaximCommon::CompileError &err) {
            std::cerr << "Error on parse: " << err.start.line << ":" << err.start.column << " -> " << err.end.line
                      << ":" << err.end.column << " :: " << err.message << std::endl;

            _errorLog.errors.push_back(err);
        }
    }
}

void CustomNode::remove() {
    for (const auto &control : _controls) {
        control->remove();
    }

    Node::remove();
}

GeneratableModuleClass *CustomNode::compile() {
    if (!_needsCompile && _moduleClass) return _moduleClass.get();
    _needsCompile = false;

    assert(_ast);

    auto oldModule = reset();

    auto oldModuleClass = std::move(_moduleClass);
    _moduleClass = std::make_unique<GeneratableModuleClass>(surface()->runtime()->ctx(), module(), "customnode");
    _scope.clear();

    try {
        // generate code
        for (const auto &statement : _ast->expressions) {
            MaximCodegen::visitExpression(_moduleClass->generate(), &_scope, statement.get());
        }
        _moduleClass->complete();

        // update control list
        updateControls();

        deploy();
    } catch (const MaximCommon::CompileError &err) {
        _errorLog.errors.push_back(err);

        std::cerr << "Error on compile: " << err.start.line << ":" << err.start.column << " -> " << err.end.line << ":"
                  << err.end.column << " :: " << err.message << std::endl;

        // revert to old moduleClass and module so we're not running half-generated code
        _moduleClass = std::move(oldModuleClass);
        setModule(std::move(oldModule));
    }

    return _moduleClass.get();
}

struct ControlUpdateVal {
    std::unique_ptr<HardControl> control;
    bool isUsed;
    bool isNew;

    ControlUpdateVal(std::unique_ptr<HardControl> control, bool isUsed, bool isNew) : control(std::move(control)),
                                                                                      isUsed(isUsed), isNew(isNew) {}
};

void CustomNode::updateControls() {
    // todo: attempt to fill controls that don't exist, with new controls

    // create index of controls that currently exist, to find which are no longer used
    std::unordered_map<MaximCodegen::ControlKey, ControlUpdateVal> currentControls;
    for (auto &control : _controls) {
        MaximCodegen::ControlKey key{control->name(), control->type()->type()};
        ControlUpdateVal updateVal(std::move(control), false, false);
        currentControls.emplace(key, std::move(updateVal));
    }
    _controls.clear();

    // loop over node controls to find which ones are used, which ones are new
    for (const auto &newControl : _scope.controls()) {
        auto pair = currentControls.find(newControl.first);

        if (pair == currentControls.end()) {
            // it's a new control
            auto genControl = HardControl::create(this, newControl.first.name, newControl.second);

            currentControls.emplace(
                newControl.first,
                ControlUpdateVal(std::move(genControl), true, true)
            );
        } else {
            // it's an old control
            pair->second.isUsed = true;
            pair->second.control->setInstance(newControl.second);
        }
    }

    std::vector<HardControl *> newControls;

    // update controls list, remove old controls
    for (auto &control : currentControls) {
        if (control.second.isNew) {
            newControls.push_back(control.second.control.get());
        }
        if (control.second.isUsed) {
            _controls.push_back(std::move(control.second.control));
        } else {
            control.second.control->remove();
        }
    }

    // add new controls - note: ordering is important here so the UI can free
    // grid space before allocating new controls
    for (const auto &newControl : newControls) {
        emit controlAdded(newControl);
    }
}
