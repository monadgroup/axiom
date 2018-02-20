#include <iostream>
#include <iomanip>
#include "NodeNumControl.h"

#include "../node/NodeSurface.h"
#include "compiler/runtime/Control.h"
#include "compiler/runtime/ControlGroup.h"
#include "compiler/runtime/Node.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

NodeNumControl::NodeNumControl(Node *node, MaximRuntime::Control *runtime, QPoint pos, QSize size)
        : NodeControl(node, runtime, pos, size), m_sink(runtime) {
    initSink();

    connect(&m_sink, &NumConnectionSink::valueChanged,
            this, &NodeNumControl::valueChanged);
}

void NodeNumControl::doRuntimeUpdate() {
    auto ptr = (uint8_t*) runtime()->group()->currentPtr();
    auto layout = runtime()->node()->runtime()->context()->numType()->layout();

    auto vecPtr = (float*)(ptr + layout->getElementOffset(0));
    auto leftNum  = *(vecPtr + 0);
    auto rightNum = *(vecPtr + 1);
    auto form = *(ptr + layout->getElementOffset(1));
    auto active = *(ptr + layout->getElementOffset(2));

    std::cout << leftNum << "," << rightNum << " " << (int32_t) form << " " << (int32_t) active << std::endl;
    std::cout << "First offset: " << layout->getElementOffset(0) << ", second offset: " << layout->getElementOffset(1) << ", third offset: " << layout->getElementOffset(2) << std::endl;

    setValue({ leftNum, rightNum, (MaximCommon::FormType) form, (bool) active });
}

void NodeNumControl::setValue(NumValue value) {
    m_sink.setValue(value);
}

void NodeNumControl::setMode(Mode mode) {
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
