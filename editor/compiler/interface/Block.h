#pragma once

#include <cstddef>
#include <variant>

#include "OwnedObject.h"
#include "ControlRef.h"
#include "Error.h"

namespace MaximCompiler {

    class Block : public OwnedObject {
    public:
        explicit Block(void *handle);

        static std::variant<Block, Error> compile(uint64_t id, const std::string &name, const std::string &code);

        size_t controlCount() const;

        ControlRef getControl(size_t index) const;
    };

}
