#pragma once

#include "NodeControl.h"
#include "../connection/ExtractConnectionSink.h"

namespace AxiomModel {

    class NodeExtractControl : public NodeControl {
    Q_OBJECT

    public:

        NodeExtractControl(Node *node, MaximRuntime::Control *runtime, ConnectionSink::Type baseType, QPoint pos,
                           QSize size);

        ExtractConnectionSink *sink() override { return &m_sink; }

        ExtractConnectionSink::ActiveSlotFlags activeSlots() const { return m_sink.activeSlots(); }

        bool isResizable() const override { return true; }

    public slots:

        void doRuntimeUpdate() override;

        // todo: implement these
        void saveValue() override {}

        void restoreValue() override {}

    signals:

        void activeSlotsChanged(ExtractConnectionSink::ActiveSlotFlags newActiveItems);

    private:

        ExtractConnectionSink m_sink;

    };

}
