#include "CustomNode.h"

#include "../schematic/Schematic.h"
#include "editor/AxiomApplication.h"

using namespace AxiomModel;

CustomNode::CustomNode(Schematic *parent, QString name, QPoint pos, QSize size)
        : Node(parent, std::move(name), Type::CUSTOM, pos, size),
          _runtime(parent->runtime()) {

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
    auto errorLog = _runtime.compile(m_code.toStdString());

    if (errorLog.errors.size()) {
        emit compileFailed(errorLog);
        return;
    }

    // todo: update control list

    AxiomApplication::runtime->rebuild();

    emit compileSucceeded();
}
