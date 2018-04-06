#pragma once

#include "compiler/common/ControlType.h"
#include "NodeControl.h"

namespace MaximRuntime {
    class IOControl;
}

namespace AxiomModel {

    class ConnectionSink;

    class IONode;

    class NodeIOControl : public NodeControl {
    Q_OBJECT

    public:

        NodeIOControl(IONode *node, size_t index, MaximRuntime::IOControl *runtime);

        ConnectionSink *sink() override { return m_sink.get(); }

        MaximCommon::ControlType ioType() const { return _ioType; }

        bool isMovable() const override { return false; }

        bool isResizable() const override { return false; }

        void doRuntimeUpdate() override {}

    public slots:

        void saveValue() override {}

        void restoreValue() override {}

    private:

        MaximCommon::ControlType _ioType;

        std::unique_ptr<ConnectionSink> m_sink;

    };

}
