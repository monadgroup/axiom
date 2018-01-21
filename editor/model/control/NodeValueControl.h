#pragma once

#include "NodeControl.h"
#include "../connection/NumConnectionSink.h"

namespace AxiomModel {

    class Node;

    class NodeValueControl : public NodeControl {
    Q_OBJECT

    public:
        enum class Type {
            BASIC,
            TOGGLE,
            LABEL
        };

        enum class Channel {
            LEFT = 1 << 0,
            RIGHT = 1 << 1,
            BOTH = LEFT | RIGHT
        };

        const Type type;

        const Channel channel;

        NodeValueControl(Node *node, QString name, Type type, Channel channel, QPoint pos, QSize size);

        NumConnectionSink *sink() const override { return m_sink.get(); }

        float value() const { return channel == Channel::RIGHT ? m_sink->value().right : m_sink->value().left; }

        bool isResizable() const override { return true; }

        std::unique_ptr<GridItem> clone(GridSurface *newParent, QPoint newPos, QSize newSize) const override;

    public slots:

        void setValue(float value);

    signals:

        void valueChanged(float newValue);

    private:

        std::unique_ptr<NumConnectionSink> m_sink = std::make_unique<NumConnectionSink>();
    };

}
