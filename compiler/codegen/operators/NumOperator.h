#pragma once

#include "../Operator.h"
#include "ActiveMode.h"

namespace MaximCodegen {

    class Num;

    class NumOperator : public Operator {
    public:
        NumOperator(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode);

        std::unique_ptr<Value> call(Builder &b, std::unique_ptr<Value> left, std::unique_ptr<Value> right) override;

        virtual std::unique_ptr<Num> call(Builder &b, Num *leftNum, Num *rightNum) = 0;

    protected:
        llvm::Value *getActive(Builder &b, Num *left, Num *right);

    private:
        ActiveMode _activeMode;
    };

}
