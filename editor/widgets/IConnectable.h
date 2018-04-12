#pragma once

namespace AxiomModel {
    class NodeControl;
}

namespace AxiomGui {

    class IConnectable {
    public:
        virtual AxiomModel::NodeControl *sink() = 0;
    };

}
