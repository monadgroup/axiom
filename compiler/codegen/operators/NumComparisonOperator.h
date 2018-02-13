#pragma once

#include <llvm/IR/Instructions.h>

#include "NumOperator.h"

namespace MaximCodegen {

    class NumComparisonOperator : public NumOperator {
    public:
        NumComparisonOperator(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode, llvm::CmpInst::Predicate op);

        static std::unique_ptr<NumComparisonOperator> create(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode, llvm::CmpInst::Predicate op);

        std::unique_ptr<Num> call(Node *node, Num *numLeft, Num *numRight) override;

    private:
        llvm::FCmpInst::Predicate _op;
    };

}
