#pragma once

#include <cstddef>
#include <variant>

#include "ControlRef.h"
#include "Error.h"
#include "OwnedObject.h"

namespace MaximCompiler {

    class Block : public OwnedObject {
    public:
        Block();

        explicit Block(void *handle);

        static bool compile(uint64_t id, const QString &name, const QString &code, Block *blockOut, Error *errorOut);

        // static std::variant<Block, Error> compile(uint64_t id, const QString &name, const QString &code);

        size_t controlCount() const;

        ControlRef getControl(size_t index) const;

        Block clone() const;
    };
}
