#pragma once

#include <memory>

#include "../SourcePos.h"

namespace llvm {
    class Value;
}

namespace MaximCodegen {

    class Type;

    class Value {
    public:
        Value(SourcePos startPos, SourcePos endPos);

        virtual llvm::Value *get() const = 0;

        virtual std::unique_ptr<Value> withSource(SourcePos startPos, SourcePos endPos) const = 0;

        virtual Type *type() const = 0;

        std::unique_ptr<Value> clone() const;

        SourcePos startPos;
        SourcePos endPos;
    };

}
