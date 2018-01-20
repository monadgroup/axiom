#pragma once

#include "NodeControl.h"

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

        const Type type;

        NodeValueControl(Node *node, Type type, Channel channel, QPoint pos, QSize size);

        float value() const { return m_value; }

        bool isResizable() const override { return true; }

    public slots:

        void setValue(float value);

    signals:

        void valueChanged(float newValue);

    private:
        float m_value = 0;
    };

}
