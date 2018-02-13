#pragma once

#include <memory>

#include "Value.h"
#include "Builder.h"
#include "../common/OperatorType.h"

namespace MaximCodegen {

    class MaximContext;

    class Operator {
    public:
        Operator(MaximContext *context, MaximCommon::OperatorType type, Type *leftType, Type *rightType);

        virtual std::unique_ptr<Value> call(Builder &b, std::unique_ptr<Value> left, std::unique_ptr<Value> right, SourcePos startPos, SourcePos endPos) = 0;

        MaximCommon::OperatorType type() const { return _type; }

        Type *leftType() const { return _leftType; }

        Type *rightType() const { return _rightType; }

    protected:
        MaximContext *context() const { return _context; }

    private:
        Type *_leftType;
        Type *_rightType;
        MaximContext *_context;
        MaximCommon::OperatorType _type;
    };

}
