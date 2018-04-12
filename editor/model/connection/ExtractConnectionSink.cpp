#include "ExtractConnectionSink.h"

using namespace AxiomModel;

ExtractConnectionSink::ExtractConnectionSink(NodeControl *control, Type baseType) : ConnectionSink(baseType, control) {

}

void ExtractConnectionSink::setActiveSlots(ActiveSlotFlags activeSlots) {
    if (activeSlots != m_activeSlots) {
        m_activeSlots = activeSlots;
        emit activeSlotsChanged(activeSlots);
    }
}
