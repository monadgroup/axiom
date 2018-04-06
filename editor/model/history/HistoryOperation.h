#pragma once

namespace AxiomModel {

    class HistoryOperation {
    public:
        explicit HistoryOperation(bool needsRefresh);

        bool needsRefresh() const { return _needsRefresh; }

        virtual void forward() = 0;

        virtual void backward() = 0;

    private:
        bool _needsRefresh;

    };

}
