#pragma once

#include "../AudioBackend.h"

namespace AxiomBackend {

    class VstAdapter {
    public:
        virtual void adapterUpdateIo() = 0;
        virtual void adapterSetParameter(size_t parameter, AxiomBackend::NumValue value) = 0;
    };

}
