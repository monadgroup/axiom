#pragma once

namespace AxiomModel {
    class ConnectionSink;
}

namespace AxiomGui {

    class IConnectable {
    public:
        virtual AxiomModel::ConnectionSink *sink() = 0;
    };

}
