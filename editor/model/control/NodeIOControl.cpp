#include "NodeIOControl.h"

#include "editor/model/node/IONode.h"
#include "../connection/NumConnectionSink.h"
#include "../connection/MidiConnectionSink.h"

using namespace AxiomModel;

NodeIOControl::NodeIOControl(IONode *node, MaximCommon::ControlType ioType, QString name)
    : NodeControl(node, std::move(name), QPoint(0, 0), QSize(2, 2)), _ioType(ioType) {
    switch (ioType) {
        case MaximCommon::ControlType::NUMBER:
            m_sink = std::make_unique<NumConnectionSink>(this);
            break;
        case MaximCommon::ControlType::MIDI:
            m_sink = std::make_unique<MidiConnectionSink>(this);
            break;
        default:
            assert(false);
    }

    initSink();
}
