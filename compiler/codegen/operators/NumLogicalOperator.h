#pragma once

#include <llvm/IR/Instruction.h>

#include "NumOperator.h"

namespace MaximCodegen {

    class NumLogicalOperator : public NumOperator {
    public:
        NumLogicalOperator(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode, llvm::Instruction::BinaryOps op);

        static std::unique_ptr<NumLogicalOperator> create(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode, llvm::Instruction::BinaryOps op);

        std::unique_ptr<Num> call(Node *node, Num *numLeft, Num *numRight) override;

    private:
        llvm::Instruction::BinaryOps _op;
    };

}
