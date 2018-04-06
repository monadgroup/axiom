#include <QtCore/QBuffer>
#include "CustomNode.h"

#include "../schematic/Schematic.h"
#include "../control/NodeControl.h"
#include "../Project.h"
#include "editor/AxiomApplication.h"
#include "compiler/runtime/Runtime.h"
#include "compiler/codegen/Control.h"
#include "../../util.h"

using namespace AxiomModel;

CustomNode::CustomNode(Schematic *parent, size_t index, QString name, QPoint pos, QSize size)
    : Node(parent, index, std::move(name), Type::CUSTOM, pos, size),
      _runtime(parent->runtime()) {
    connect(&_runtime, &MaximRuntime::CustomNode::controlAdded,
            this, &CustomNode::controlAdded);
    connect(&_runtime, &MaximRuntime::CustomNode::extractedChanged,
            this, &CustomNode::extractedChanged);

    connect(this, &CustomNode::removed,
            [this]() {
                _runtime.remove();
            });
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

        _runtime.setCode(code.toStdString());
        if (!_runtime.errorLog().errors.empty()) {
            emit parseFailed(_runtime.errorLog());
            _runtime.errorLog().errors.clear();
        } else {
            emit parseSucceeded();
        }
    }
}

void CustomNode::recompile() {
    parentSchematic->project()->build();

    if (!_runtime.errorLog().errors.empty()) {
        emit compileFailed(_runtime.errorLog());
        _runtime.errorLog().errors.clear();
    } else {
        emit compileSucceeded();
    }

    emit compileFinished();
}

void CustomNode::serialize(QDataStream &stream) const {
    Node::serialize(stream);
    stream << m_code;

    // serialize controls
    stream << (uint32_t) surface.items().size();
    for (const auto &item : surface.items()) {
        auto control = dynamic_cast<NodeControl*>(item.get());
        assert(control);

        stream << control->name();
        stream << (uint8_t) control->runtime()->type()->type();

        // we need to write the size of the control data so the deserializer can skip it
        // so serialize the control into a memory buffer before putting it into the main
        // one.
        QBuffer controlBuffer;
        controlBuffer.open(QBuffer::WriteOnly);
        QDataStream dataStream(&controlBuffer);
        control->serialize(dataStream);

        stream << (uint64_t) controlBuffer.size();
        stream.writeRawData(controlBuffer.data(), (int) controlBuffer.size());
        controlBuffer.close();
    }
}

void CustomNode::deserialize(QDataStream &stream) {
    Node::deserialize(stream);

    QString code; stream >> code;
    setCode(code);

    // trigger the runtime to create controls for us, which will create controls on our surface
    _runtime.scheduleCompile();
    _runtime.compile();

    // create an index of name/type to control for deserialization
    std::unordered_map<MaximCodegen::ControlKey, NodeControl*> controlMap;
    for (const auto &item : surface.items()) {
        if (auto control = dynamic_cast<NodeControl*>(item.get())) {
            controlMap.emplace(MaximCodegen::ControlKey {
                control->name().toStdString(),
                control->runtime()->type()->type()
            }, control);
        }
    }

    // deserialize controls
    uint32_t controlCount; stream >> controlCount;
    for (uint32_t i = 0; i < controlCount; i++) {
        QString controlName; stream >> controlName;
        uint8_t intControlType; stream >> intControlType;

        uint64_t controlSize; stream >> controlSize;

        auto controlType = (MaximCommon::ControlType) intControlType;
        MaximCodegen::ControlKey controlKey = { controlName.toStdString(), controlType };
        auto realControlIndex = controlMap.find(controlKey);

        // if the {name, type} pair doesn't exist anymore, skip the controls data
        if (realControlIndex == controlMap.end()) {
            stream.skipRawData((int) controlSize);
            continue;
        }

        NodeControl *control = realControlIndex->second;
        control->deserialize(stream);
    }
}

void CustomNode::controlAdded(MaximRuntime::HardControl *control) {
    surface.addItem(NodeControl::fromRuntimeControl(this, surface.items().size(), control));
}
