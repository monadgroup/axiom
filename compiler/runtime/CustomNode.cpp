#include "CustomNode.h"

#include "../parser/TokenStream.h"
#include "../parser/Parser.h"
#include "../ast/Block.h"
#include "../codegen/Value.h"
#include "Runtime.h"

using namespace MaximRuntime;

CustomNode::CustomNode(Schematic *parent) : Node(parent), _node(parent->runtime()->context(), module()) {
    setCode("");
}

CustomNode::~CustomNode() = default;

void CustomNode::setCode(const std::string &code) {
    if (code != _code) {
        _code = code;

        _errorLog.errors.clear();

        try {
            auto stream = std::make_unique<MaximParser::TokenStream>(_code);
            MaximParser::Parser parser(std::move(stream));
            _ast = std::move(parser.parse());

            scheduleCompile();
        } catch (const MaximCommon::CompileError &err) {
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

void CustomNode::compile() {
    assert(_ast);

    inst()->reset();

    _errorLog.errors.clear();

    try {
        // generate code
        _node.reset();
        _node.generateCode(_ast.get());
        _node.complete();

        // update control list
        updateControls();

        auto ptr = inst()->addInstantiable(&_node);
        MaximCodegen::CreateCall(inst()->builder(), _node.generateFunc(module()), {ptr}, "");
    } catch (const MaximCommon::CompileError &err) {
        _errorLog.errors.push_back(err);

        // clear flag for deploy, since we want to keep the old module loaded
        // note: this won't clear parent CompileUnit's deploy flags, so they will still deploy unnecessarily
        // would be good to fix this, but it's not too important
        cancelDeploy();
    }

    inst()->complete();
    CompileUnit::compile();
}

struct ControlUpdateVal {
    std::unique_ptr<HardControl> control;
    bool isUsed;
    bool isNew;

    ControlUpdateVal(std::unique_ptr<HardControl> control, bool isUsed, bool isNew) : control(std::move(control)),
                                                                                      isUsed(isUsed), isNew(isNew) {}
};

void CustomNode::updateControls() {
    // create index of controls that currently exist, to find which are no longer used
    std::unordered_map<MaximCodegen::ControlKey, ControlUpdateVal> currentControls;
    for (auto &control : _controls) {
        MaximCodegen::ControlKey key{control->name(), control->type()};
        ControlUpdateVal updateVal(std::move(control), false, false);
        currentControls.emplace(key, std::move(updateVal));
    }
    _controls.clear();

    // loop over node controls to find which ones are used, which ones are new
    for (const auto &newControl : _node.controls()) {
        auto pair = currentControls.find(newControl.first);

        if (pair == currentControls.end()) {
            // it's a new control
            auto genControl = HardControl::create(this, newControl.first.name, newControl.second.control);

            currentControls.emplace(
                newControl.first,
                ControlUpdateVal(std::move(genControl), true, true)
            );
        } else {
            // it's an old control
            pair->second.isUsed = true;
            pair->second.control->setControl(newControl.second.control);
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
