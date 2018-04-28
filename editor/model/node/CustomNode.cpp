#include "CustomNode.h"

#include <QtCore/QBuffer>
#include <iostream>

#include "../schematic/Schematic.h"
#include "../control/NodeControl.h"
#include "../history/ChangeCodeOperation.h"
#include "../Project.h"
#include "editor/AxiomApplication.h"
#include "compiler/runtime/Runtime.h"
#include "../../util.h"

using namespace AxiomModel;

CustomNode::CustomNode(Schematic *parent, QString name, QPoint pos, QSize size)
    : Node(parent, std::move(name), Type::CUSTOM, pos, size) {
}

void CustomNode::attachRuntime(MaximRuntime::CustomNode *runtime) {
    assert(!_runtime);
    _runtime = runtime;

    connect(_runtime, &MaximRuntime::CustomNode::controlAdded,
            this, &CustomNode::controlAdded);
    connect(_runtime, &MaximRuntime::CustomNode::extractedChanged,
            this, &CustomNode::extractedChanged);
    connect(_runtime, &MaximRuntime::CustomNode::finishedCodegen,
            this, &CustomNode::finishedCodegen);

    connect(this, &CustomNode::removed,
            [this]() {
                _runtime->remove();
            });


    // add any controls that might already exist
    for (const std::unique_ptr<MaximRuntime::Control> &control : *runtime) {
        controlAdded(control.get());
    }

    parseCode();
}

void CustomNode::createAndAttachRuntime(MaximRuntime::Surface *surface) {
    auto runtime = std::make_unique<MaximRuntime::CustomNode>(surface);
    attachRuntime(runtime.get());
    surface->addNode(std::move(runtime));
}

std::unique_ptr<GridItem> CustomNode::clone(GridSurface *newParent, QPoint newPos, QSize newSize) const {
    /*auto schematicParent = dynamic_cast<Schematic *>(newParent);
    assert(schematicParent != nullptr);

    auto customNode = std::make_unique<CustomNode>(schematicParent, name(), pos(), size());
    return std::move(customNode);*/
    unreachable;
}

void CustomNode::setCode(const QString &code) {
    if (code != m_code) {
        m_code = code;
        emit codeChanged(code);

        parseCode();
    }
}

void CustomNode::parseCode() {
    if (_runtime) {
        _runtime->setCode(m_code.toStdString());
        if (!_runtime->errorLog().errors.empty()) {
            emit parseFailed(_runtime->errorLog());
            _runtime->errorLog().errors.clear();
        } else {
            emit parseSucceeded();
        }
    }
}

void CustomNode::recompile() {
    if (lastCode != m_code) {
        parentSchematic->project()->history.appendOperation(
            std::make_unique<ChangeCodeOperation>(parentSchematic->project(), ref(), lastCode, m_code));
        lastCode = m_code;
    }
}

void CustomNode::serialize(QDataStream &stream, QPoint offset) const {
    Node::serialize(stream, offset);
    stream << m_code;

    // serialize controls
    stream << (uint32_t) surface.items().size();
    for (const auto &item : surface.items()) {
        auto control = dynamic_cast<NodeControl *>(item.get());
        assert(control);

        stream << control->name();
        stream << (uint8_t) control->type();
        control->serialize(stream, QPoint(0, 0));
    }
}

void CustomNode::deserialize(QDataStream &stream, QPoint offset) {
    Node::deserialize(stream, offset);

    QString code;
    stream >> code;
    setCode(code);

    uint32_t controlCount;
    stream >> controlCount;
    for (uint32_t i = 0; i < controlCount; i++) {
        QString controlName;
        stream >> controlName;
        uint8_t intControlType;
        stream >> intControlType;

        auto control = NodeControl::create(this, (MaximCommon::ControlType) intControlType, controlName);
        control->deserialize(stream, QPoint(0, 0));
        surface.addItem(std::move(control));
    }
}

void CustomNode::controlAdded(MaximRuntime::Control *control) {
    addFromRuntime(control);
}

void CustomNode::finishedCodegen() {
    if (_runtime) {
        std::cout << "Finished codegen! Error count: " << _runtime->errorLog().errors.size() << std::endl;

        if (!_runtime->errorLog().errors.empty()) {
            emit compileFailed(_runtime->errorLog());
            _runtime->errorLog().errors.clear();
        } else {
            emit compileSucceeded();
        }
    }

    emit compileFinished();
}
