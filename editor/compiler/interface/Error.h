#pragma once

#include <string>

#include "OwnedObject.h"
#include "Frontend.h"

namespace MaximCompiler {

    class Error : public OwnedObject {
    public:
        explicit Error(void *handle);

        std::string getDescription() const;
        MaximFrontend::SourceRange getRange() const;
    };

}
