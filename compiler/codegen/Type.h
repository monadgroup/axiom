#pragma once

#include <memory>

#include "../SourcePos.h"

namespace llvm {
    class Type;
}

namespace MaximCodegen {

    class Value;

    class Type {
    public:
        virtual llvm::Type *get() const = 0;

        virtual std::string name() const = 0;

        virtual std::unique_ptr<Value> createInstance(llvm::Value *val, SourcePos startPos, SourcePos endPos);
    };

}
