#include "Operator.h"

using namespace MaximCodegen;

Operator::Operator(MaximContext *context, MaximCommon::OperatorType type, Type *leftType, Type *rightType)
    : _context(context), _type(type), _leftType(leftType), _rightType(rightType) {
}
