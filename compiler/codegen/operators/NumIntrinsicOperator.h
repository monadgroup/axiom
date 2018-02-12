#pragma once

#include <llvm/IR/Intrinsics.h>

#include "NumOperator.h"

namespace llvm {
    class Module;
}

namespace MaximCodegen {

    class NumIntrinsicOperator : public NumOperator {
    public:
        NumIntrinsicOperator(MaximContext *context, llvm::Module *module, MaximCommon::OperatorType type, ActiveMode activeMode, llvm::Intrinsic::ID id);

        static std::unique_ptr<NumIntrinsicOperator> create(MaximContext *context, llvm::Module *module, MaximCommon::OperatorType type, ActiveMode activeMode, llvm::Intrinsic::ID id);

        std::unique_ptr<Num> call(Builder &b, Num *numLeft, Num *numRight) override;

    private:
        llvm::Module *_module;
        llvm::Intrinsic::ID _id;
    };

}
