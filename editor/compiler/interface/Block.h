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

        static std::variant<Block, Error> compile(uint64_t id, const QString &name, const QString &code);

        size_t controlCount() const;

        ControlRef getControl(size_t index) const;

        Block clone() const;
    };

}
