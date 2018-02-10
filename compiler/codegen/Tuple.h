#pragma once

#include <memory>
#include <vector>

#include "Value.h"

namespace llvm {
    class Value;
}

namespace MaximCodegen {

    class TupleType;

    class Tuple : public Value {
    public:
        using Storage = std::vector<std::unique_ptr<Value>>;

        Tuple(Storage values);

        static std::unique_ptr<Tuple> create(Storage values);

        Value *atIndex(size_t index) const;

        TupleType *type();
    };

}
