#pragma once

#include "../SourcePos.h"

namespace MaximCodegen {

    class Value {
    public:
        Value(SourcePos startPos, SourcePos endPos);

        virtual llvm::Value *get() const = 0;

        virtual std::unique_ptr<Value> withSource(SourcePos startPos, SourcePos endPos) const = 0;

        std::unique_ptr<Value> clone() const;

        llvm::Type *type() const;

        SourcePos startPos;
        SourcePos endPos;
    };

}
