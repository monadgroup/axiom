#include "NodeIOControl.h"

#include "compiler/runtime/IOControl.h"
#include "editor/model/node/IONode.h"
#include "../connection/NumConnectionSink.h"
#include "../connection/MidiConnectionSink.h"

using namespace AxiomModel;

NodeIOControl::NodeIOControl(IONode *node, size_t index, MaximRuntime::IOControl *runtime)
    : NodeControl(node, index, runtime, QPoint(0, 0), QSize(2, 2)), _ioType(runtime->ioType()) {
    switch (runtime->ioType()) {
        case MaximCommon::ControlType::NUMBER:
            m_sink = std::make_unique<NumConnectionSink>(runtime);
            break;
        case MaximCommon::ControlType::MIDI:
            m_sink = std::make_unique<MidiConnectionSink>(runtime);
            break;
        default:
            assert(false);
    }

    initSink();
}
