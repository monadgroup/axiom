#pragma once

#include <llvm/IR/Intrinsics.h>

#include "NumOperator.h"

namespace llvm {
    class Module;
}

namespace MaximCodegen {

    class NumIntrinsicOperator : public NumOperator {
    public:
        NumIntrinsicOperator(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode,
                             llvm::Intrinsic::ID id);

        static std::unique_ptr<NumIntrinsicOperator>
        create(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode, llvm::Intrinsic::ID id);

        std::unique_ptr<Num> call(Node *node, Num *numLeft, Num *numRight) override;

    private:
        llvm::Intrinsic::ID _id;
    };

}
