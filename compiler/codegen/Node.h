#pragma once

#include <memory>

#include "Instantiable.h"
#include "Builder.h"

namespace MaximCodegen {

    class MaximContext;

    class Value;

    class Node : public Instantiable {
    public:
        MaximContext *ctx() const;

        Builder &builder();

        Value *getVariable(std::string name);

        void setVariable(std::string name, std::unique_ptr<Value> value);

        void setAssignable(MaximAst::AssignableExpression *assignable, std::unique_ptr<Value> value);

        llvm::Value *addInstantiable(std::unique_ptr<Instantiable> inst);
    };

}
