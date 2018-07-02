#pragma once

#include "common/Event.h"
#include "Control.h"

namespace AxiomModel {

    class ExtractControl : public Control {
    public:
        using ActiveSlotFlags = uint16_t;

        AxiomCommon::Event<ActiveSlotFlags> activeSlotsChanged;

        ExtractControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                       bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                       ConnectionWire::WireType wireType, ActiveSlotFlags activeSlots, ModelRoot *root);

        static std::unique_ptr<ExtractControl> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos,
                                                      QSize size, bool selected, QString name, bool showName,
                                                      const QUuid &exposerUuid, const QUuid &exposingUuid,
                                                      ConnectionWire::WireType wireType, ActiveSlotFlags activeSlots,
                                                      ModelRoot *root);

        static std::unique_ptr<ExtractControl>
        deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                    bool selected, QString name, bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                    ConnectionWire::WireType wireType, ReferenceMapper *ref, ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        ActiveSlotFlags activeSlots() const { return _activeSlots; }

        void setActiveSlots(ActiveSlotFlags activeSlots);

        // todo: implement these?
        void saveValue() override { }

        void restoreValue() override { }

    private:
        ActiveSlotFlags _activeSlots;
    };

}
