#pragma once

#include <llvm/IR/Instruction.h>

#include "NumOperator.h"

namespace MaximCodegen {

    class NumFloatOperator : public NumOperator {
    public:
        NumFloatOperator(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode,
                         llvm::Instruction::BinaryOps op);

        static std::unique_ptr<NumFloatOperator>
        create(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode,
               llvm::Instruction::BinaryOps op);

        std::unique_ptr<Num> call(Node *node, Num *numLeft, Num *numRight) override;

    private:
        llvm::Instruction::BinaryOps _op;
    };

}
