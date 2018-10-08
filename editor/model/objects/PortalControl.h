#pragma once

#include "Control.h"

namespace AxiomModel {

    class PortalControl : public Control {
    public:
        enum class PortalType { INPUT, OUTPUT, AUTOMATION };

        AxiomCommon::Event<> labelWillChange;
        AxiomCommon::Event<const QString &> labelChanged;

        PortalControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                      bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                      ConnectionWire::WireType wireType, PortalType portalType, uint64_t portalId, ModelRoot *root);

        static std::unique_ptr<PortalControl> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                     bool selected, QString name, bool showName,
                                                     const QUuid &exposerUuid, const QUuid &exposingUuid,
                                                     ConnectionWire::WireType wireType, PortalType portalType,
                                                     uint64_t portalId, ModelRoot *root);

        bool isMovable() const override { return false; }

        PortalType portalType() const { return _portalType; }

        uint64_t portalId() const { return _portalId; }

        void doRuntimeUpdate() override {}

        void restoreState() override;

        const QString &portalLabel() const { return _portalLabel; }

        void setPortalLabel(QString portalLabel);

    private:
        PortalType _portalType;
        uint64_t _portalId;
        QString _portalLabel;
    };
}
