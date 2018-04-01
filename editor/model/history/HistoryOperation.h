#pragma once

namespace AxiomModel {

    class HistoryOperation {
    public:

        virtual void forward() = 0;

        virtual void backward() = 0;

    };

}
