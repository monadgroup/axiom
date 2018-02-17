#include "CustomNode.h"

#include "../schematic/Schematic.h"
#include "../control/NodeControl.h"
#include "editor/AxiomApplication.h"
#include "compiler/runtime/Control.h"

using namespace AxiomModel;

CustomNode::CustomNode(Schematic *parent, QString name, QPoint pos, QSize size)
        : Node(parent, std::move(name), Type::CUSTOM, pos, size),
          _runtime(parent->runtime()) {
    parent->runtime()->addNode(&_runtime);
}

CustomNode::~CustomNode() {
    parentSchematic->runtime()->removeNode(&_runtime);
}

std::unique_ptr<GridItem> CustomNode::clone(GridSurface *newParent, QPoint newPos, QSize newSize) const {
    auto schematicParent = dynamic_cast<Schematic *>(newParent);
    assert(schematicParent != nullptr);

    auto customNode = std::make_unique<CustomNode>(schematicParent, name(), pos(), size());
    surface.cloneTo(&customNode->surface);
    return std::move(customNode);
}

void CustomNode::setCode(const QString &code) {
    if (code != m_code) {
        m_code = code;
        emit codeChanged(code);

        recompile();
    }
}

void CustomNode::recompile() {
    // ensure removed controls are kept around until the end of the function
    std::vector<std::unique_ptr<MaximRuntime::Control>> removedControls;
    auto errorLog = _runtime.compile(m_code.toStdString(), removedControls);

    if (errorLog.errors.size()) {
        emit compileFailed(errorLog);
        return;
    }

    // remove controls marked as deleted
    for (const auto &item : surface.items()) {
        if (auto control = dynamic_cast<NodeControl*>(item.get())) {
            if (control->runtime()->isDeleted) {
                control->remove();
            }
        }
    }

    // add new controls
    for (const auto &runtimeControl : _runtime.controls()) {
        if (runtimeControl->isNew) {
            surface.addItem(NodeControl::fromRuntimeControl(this, runtimeControl.get()));
        }
    }

    AxiomApplication::runtime->rebuild();

    emit compileSucceeded();
}
