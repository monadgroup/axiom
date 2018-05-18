#include "CustomNode.h"

#include <iostream>

#include "ControlSurface.h"
#include "NumControl.h"
#include "MidiControl.h"
#include "ExtractControl.h"
#include "compiler/runtime/Surface.h"
#include "compiler/runtime/CustomNode.h"
#include "compiler/codegen/Control.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../actions/CreateControlAction.h"
#include "../../util.h"

using namespace AxiomModel;

CustomNode::CustomNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                       const QUuid &controlsUuid, QString code, bool panelOpen, float panelHeight,
                       AxiomModel::ModelRoot *root)
    : Node(NodeType::CUSTOM_NODE, uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, root),
      _code(std::move(code)), _isPanelOpen(panelOpen), _panelHeight(panelHeight) {
}

std::unique_ptr<CustomNode> CustomNode::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                               bool selected, QString name, const QUuid &controlsUuid, QString code,
                                               bool panelOpen, float panelHeight, AxiomModel::ModelRoot *root) {
    return std::make_unique<CustomNode>(uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, code,
                                        panelOpen, panelHeight, root);
}

std::unique_ptr<CustomNode> CustomNode::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                    QPoint pos, QSize size, bool selected, QString name,
                                                    const QUuid &controlsUuid, AxiomModel::ModelRoot *root) {
    QString code;
    stream >> code;
    bool isPanelOpen;
    stream >> isPanelOpen;
    float panelHeight;
    stream >> panelHeight;

    return create(uuid, parentUuid, pos, size, selected, std::move(name), controlsUuid, code, isPanelOpen, panelHeight,
                  root);
}

void CustomNode::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    Node::serialize(stream, parent, withContext);
    stream << _code;
    stream << _isPanelOpen;
    stream << _panelHeight;
}

void CustomNode::setCode(const QString &code) {
    if (_code != code) {
        _code = code;
        codeChanged.trigger(code);

        if (_runtime) (*_runtime)->setCode(code.toStdString());
    }
}

void CustomNode::setPanelOpen(bool panelOpen) {
    if (_isPanelOpen != panelOpen) {
        _isPanelOpen = panelOpen;
        panelOpenChanged.trigger(panelOpen);
    }
}

void CustomNode::setPanelHeight(float panelHeight) {
    if (panelHeight < minPanelHeight) panelHeight = minPanelHeight;
    if (_panelHeight != panelHeight) {
        beforePanelHeightChanged.trigger(panelHeight);
        _panelHeight = panelHeight;
        panelHeightChanged.trigger(panelHeight);
    }
}

void CustomNode::createAndAttachRuntime(MaximRuntime::Surface *parent) {
    auto runtime = std::make_unique<MaximRuntime::CustomNode>(parent);
    attachRuntime(runtime.get());
    parent->addNode(std::move(runtime));
}

void CustomNode::attachRuntime(MaximRuntime::CustomNode *runtime) {
    assert(!_runtime);
    _runtime = runtime;

    runtime->controlAdded.connect(this, &CustomNode::runtimeAddedControl);
    runtime->extractedChanged.connect(this, &CustomNode::setExtracted);
    runtime->finishedCodegen.connect(this, &CustomNode::runtimeFinishedCodegen);

    controls().then([this](ControlSurface *const &controls) { controls->controls().itemAdded.connect(this, &CustomNode::surfaceControlAdded); });

    removed.connect(this, &CustomNode::detachRuntime);

    // add any controls that might already exist in the runtime
    for (const auto &control : *runtime) {
        runtimeAddedControl(control.get());
    }
}

void CustomNode::detachRuntime() {
    if (_runtime) (*_runtime)->remove();
    _runtime.reset();
}

void CustomNode::runtimeAddedControl(MaximRuntime::Control *control) {
    // note: CustomNodes rely on the runtime to create and destroy controls, as controls are based on
    // the compiled code. This is different to GroupNodes, whose controls are never created or destroyed
    // by the runtime but rather attached and detached to it when necessary.

    // find a control that we can attach to
    if (controls().value()) {
        auto controlList = (*controls().value())->controls();
        for (const auto &existingControl : controlList) {
            if (!existingControl->canAttachRuntime(control)) continue;

            existingControl->attachRuntime(control);
            control->removed.connect(existingControl, &Control::remove);
            return;
        }
    }

    // no control found, we need to create a new one
    // note: this will trigger `surfaceControlAdded`, which will attach a runtime to the control
    CreateControlAction::create((*controls().value())->uuid(), Control::fromRuntimeType(control->type()->type()), QString::fromStdString(control->name()), root())->forward(true);
}

void CustomNode::runtimeFinishedCodegen() {
    // todo
}

void CustomNode::surfaceControlAdded(AxiomModel::Control *control) {
    if (!_runtime) return;

    // find a runtime control to attach
    for (const std::unique_ptr<MaximRuntime::Control> &runtimeControl : **_runtime) {
        if (!control->canAttachRuntime(runtimeControl.get())) continue;

        control->attachRuntime(runtimeControl.get());
        runtimeControl->removed.connect(control, &Control::remove);
        return;
    }
    unreachable;
}
