#include "CustomNode.h"

#include "../schematic/Schematic.h"
#include "../control/NodeControl.h"
#include "editor/AxiomApplication.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

CustomNode::CustomNode(Schematic *parent, QString name, QPoint pos, QSize size)
    : Node(parent, std::move(name), Type::CUSTOM, pos, size),
      _runtime(parent->runtime()) {
    connect(&_runtime, &MaximRuntime::CustomNode::controlAdded,
            this, &CustomNode::controlAdded);

    connect(this, &CustomNode::removed,
            [this]() {
                _runtime.remove();
                _runtime.runtime()->compileAndDeploy();
            });
}

std::unique_ptr<GridItem> CustomNode::clone(GridSurface *newParent, QPoint newPos, QSize newSize) const {
    auto schematicParent = dynamic_cast<Schematic *>(newParent);
    assert(schematicParent != nullptr);

    auto customNode = std::make_unique<CustomNode>(schematicParent, name(), pos(), size());
    return std::move(customNode);
}

void CustomNode::setCode(const QString &code) {
    if (code != m_code) {
        m_code = code;
        emit codeChanged(code);

        _runtime.setCode(code.toStdString());
        if (!_runtime.errorLog().errors.empty()) {
            emit parseFailed(_runtime.errorLog());
        } else {
            emit parseSucceeded();
        }
    }
}

void CustomNode::recompile() {
    _runtime.compile();
    _runtime.runtime()->compileAndDeploy();

    if (!_runtime.errorLog().errors.empty()) {
        emit compileFailed(_runtime.errorLog());
    } else {
        emit compileSucceeded();
    }
}

void CustomNode::controlAdded(MaximRuntime::HardControl *control) {
    surface.addItem(NodeControl::fromRuntimeControl(this, control));
}
