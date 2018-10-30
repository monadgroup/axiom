#pragma once

#include "Control.h"
#include "common/Event.h"

namespace AxiomModel {

    class ExtractControl : public Control {
    public:
        using ActiveSlotFlags = uint32_t;

        AxiomCommon::Event<ActiveSlotFlags> activeSlotsChanged;

        ExtractControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                       bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                       ConnectionWire::WireType wireType, ActiveSlotFlags activeSlots, ModelRoot *root);

        static std::unique_ptr<ExtractControl> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos,
                                                      QSize size, bool selected, QString name, bool showName,
                                                      const QUuid &exposerUuid, const QUuid &exposingUuid,
                                                      ConnectionWire::WireType wireType, ActiveSlotFlags activeSlots,
                                                      ModelRoot *root);

        QString debugName() override;

        ActiveSlotFlags activeSlots() const { return _activeSlots; }

        void setActiveSlots(ActiveSlotFlags activeSlots);

        void doRuntimeUpdate() override;

    private:
        ActiveSlotFlags _activeSlots;
    };
}
