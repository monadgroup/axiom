#include <editor/model/node/Node.h>
#include "NodeNumControl.h"

#include "../schematic/Schematic.h"
#include "../node/NodeSurface.h"
#include "../Project.h"
#include "../history/ChangeNumValueOperation.h"
#include "../history/ChangeNumModeOperation.h"
#include "compiler/runtime/Control.h"
#include "compiler/runtime/ControlGroup.h"
#include "compiler/runtime/Node.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

NodeNumControl::NodeNumControl(Node *node, QString name, QPoint pos, QSize size)
    : NodeControl(node, std::move(name), pos, size), m_sink(this) {
    initSink();

    connect(&m_sink, &NumConnectionSink::valueChanged,
            this, &NodeNumControl::valueChanged);
}

void NodeNumControl::doRuntimeUpdate() {
    saveValue();
}

void NodeNumControl::startSetValue() {
    beforeVal = value();
}

void NodeNumControl::setValue(MaximRuntime::NumValue value, bool setRuntime) {
    m_sink.setValue(value);
    if (setRuntime) restoreValue();
}

void NodeNumControl::endSetValue() {
    if (beforeVal != value()) {
        node->parentSchematic->project()->history.appendOperation(
            std::make_unique<ChangeNumValueOperation>(node->parentSchematic->project(), ref(), beforeVal, value()));
    }
}

void NodeNumControl::saveValue() {
    if (!runtime() || !runtime()->group()) return;
    setValue(runtime()->group()->getNumValue(), false);
}

void NodeNumControl::restoreValue() {
    if (!runtime() || !runtime()->group()) return;
    runtime()->group()->setNumValue(m_sink.value());
}

void NodeNumControl::setMode(Mode mode) {
    if (mode != m_mode) {
        node->parentSchematic->project()->history.appendOperation(
            std::make_unique<ChangeNumModeOperation>(node->parentSchematic->project(), ref(), m_mode, mode));
    }
}

void NodeNumControl::setModeNoOp(AxiomModel::NodeNumControl::Mode mode) {
    if (mode != m_mode) {
        m_mode = mode;
        emit modeChanged(mode);
    }
}

void NodeNumControl::setChannel(Channel channel) {
    if (channel != m_channel) {
        m_channel = channel;
        emit channelChanged(channel);
    }
}

void NodeNumControl::serialize(QDataStream &stream) const {
    stream << m_sink.value();
    stream << (uint8_t) mode();
}

void NodeNumControl::deserialize(QDataStream &stream) {
    MaximRuntime::NumValue val; stream >> val;
    m_sink.setValue(val);

    uint8_t modeInt; stream >> modeInt;
    setModeNoOp((Mode) modeInt);
}
