#pragma once

#include <string>

#include "OwnedObject.h"
#include "SurfaceRef.h"

namespace MaximCompiler {

    class Transaction : public OwnedObject {
    public:
        Transaction();

        SurfaceRef buildSurface(uint64_t id, const std::string &name);
    };

}
