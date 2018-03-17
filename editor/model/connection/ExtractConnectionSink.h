#pragma once

#include "ConnectionSink.h"

namespace AxiomModel {

    class ExtractConnectionSink : public ConnectionSink {
        Q_OBJECT

    public:
        using ActiveSlotFlags = uint16_t;

        explicit ExtractConnectionSink(MaximRuntime::Control *runtime, Type baseType);

        ActiveSlotFlags activeSlots() const { return m_activeSlots; }

    public slots:

        void setActiveSlots(ActiveSlotFlags activeSlots);

    signals:

        void activeSlotsChanged(ActiveSlotFlags newActiveSlots);

    private:

        ActiveSlotFlags m_activeSlots;

    };

}
