#include "ExtractConnectionSink.h"

using namespace AxiomModel;

ExtractConnectionSink::ExtractConnectionSink(MaximRuntime::Control *runtime, Type baseType) : ConnectionSink(baseType, runtime) {

}

void ExtractConnectionSink::setActiveSlots(ActiveSlotFlags activeSlots) {
    if (activeSlots != m_activeSlots) {
        m_activeSlots = activeSlots;
        emit activeSlotsChanged(activeSlots);
    }
}
