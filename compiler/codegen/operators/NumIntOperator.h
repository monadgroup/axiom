#pragma once

#include <llvm/IR/Instruction.h>

#include "NumOperator.h"

namespace MaximCodegen {

    class NumIntOperator : public NumOperator {
    public:
        NumIntOperator(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode,
                       llvm::Instruction::BinaryOps op, bool isSigned);

        static std::unique_ptr<NumIntOperator>
        create(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode,
               llvm::Instruction::BinaryOps op, bool isSigned);

        std::unique_ptr<Num> call(Node *node, Num *numLeft, Num *numRight) override;

    private:
        llvm::Instruction::BinaryOps _op;
        bool _isSigned;
    };

}
