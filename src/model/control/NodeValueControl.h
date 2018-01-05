#pragma once

#include "NodeControl.h"

namespace AxiomModel {

    class NodeValueControl : public NodeControl {
        Q_OBJECT

    public:
        enum class Type {
            KNOB,
            SLIDER,
            SWITCH
        };

        const Type type;

        NodeValueControl(NodeSurface *parent, Type type, Channel channel);

        float value() const { return m_value; }

    public slots:

        void setValue(float value);

    signals:

        void valueChanged(float newValue);

    private:
        float m_value = 0;
    };

}
