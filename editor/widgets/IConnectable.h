#pragma once

namespace AxiomModel {
    class Control;
}

namespace AxiomGui {

    class IConnectable {
    public:
        virtual AxiomModel::Control *sink() = 0;
    };
}
