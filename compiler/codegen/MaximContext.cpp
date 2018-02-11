#include "MaximContext.h"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>

#include "Num.h"
#include "Midi.h"
#include "Tuple.h"
#include "Function.h"
#include "Operator.h"
#include "Converter.h"

using namespace MaximCodegen;

MaximContext::MaximContext() : _numType(this), _midiType(this) {

}

void MaximContext::assertType(Value *val, Type *type) const {
    if (val->type() != type) {
        throw typeAssertFailed(type, val->type(), val->startPos, val->endPos);
    }
}

std::unique_ptr<Num> MaximContext::assertNum(std::unique_ptr<Value> val) const {
    auto res = assertNum(val.get());
    val.release();
    return std::unique_ptr<Num>(res);
}

Num* MaximContext::assertNum(Value *val) const {
    if (auto res = dynamic_cast<Num*>(val)) return res;
    throw typeAssertFailed(numType(), val->type(), val->startPos, val->endPos);
}

std::unique_ptr<Midi> MaximContext::assertMidi(std::unique_ptr<Value> val) const {
    auto res = assertMidi(val.get());
    val.release();
    return std::unique_ptr<Midi>(res);
}

Midi* MaximContext::assertMidi(Value *val) const {
    if (auto res = dynamic_cast<Midi*>(val)) return res;
    throw typeAssertFailed(midiType(), val->type(), val->startPos, val->endPos);
}

std::unique_ptr<Tuple> MaximContext::assertTuple(std::unique_ptr<Value> val, TupleType *type) const {
    auto res = assertTuple(val.get(), type);
    val.release();
    return std::unique_ptr<Tuple>(res);
}

Tuple* MaximContext::assertTuple(Value *val, TupleType *type) const {
    if (val->type() == type) {
        if (auto res = dynamic_cast<Tuple *>(val)) {
            return res;
        }
    }
    throw typeAssertFailed(type, val->type(), val->startPos, val->endPos);
}

TupleType* MaximContext::getTupleType(const std::vector<Type *> &types) {
    std::vector<llvm::Type *> llTypes;
    llTypes.reserve(types.size());
    for (const auto &type : types) {
        llTypes.push_back(type->get());
    }
    auto structType = llvm::StructType::get(_llvm, llTypes);
    auto mapIndex = tupleTypeMap.find(structType);

    if (mapIndex == tupleTypeMap.end()) {
        return &tupleTypeMap.emplace(structType, TupleType(this, types, structType)).first->second;
    } else {
        return &mapIndex->second;
    }
}

llvm::Constant* MaximContext::constFloat(float num) const {
    return llvm::ConstantFP::get(llvm::Type::getFloatTy(_llvm), num);
}

llvm::Constant* MaximContext::constInt(unsigned int numBits, uint64_t val, bool isSigned) const {
    return llvm::ConstantInt::get(llvm::Type::getIntNTy(_llvm, numBits), val, isSigned);
}

void MaximContext::registerOperator(MaximCommon::OperatorType type, std::unique_ptr<Operator> op) {
    OperatorKey key = { type, op->leftType(), op->rightType() };
    operatorMap.emplace(key, std::move(op));
}

void MaximContext::registerFunction(std::string name, std::unique_ptr<Function> func) {
    getOrCreateFunctionList(std::move(name)).push_back(std::move(func));
}

void MaximContext::registerConverter(MaximCommon::FormType destType, std::unique_ptr<Converter> con) {
    converterMap.emplace(destType, std::move(con));
}

Operator* MaximContext::getOperator(MaximCommon::OperatorType type, Type *leftType, Type *rightType) {
    OperatorKey key = { type, leftType, rightType };
    auto op = operatorMap.find(key);
    if (op == operatorMap.end()) return nullptr;
    return op->second.get();
}

std::unique_ptr<Value> MaximContext::callOperator(MaximCommon::OperatorType type, std::unique_ptr<Value> leftVal,
                                                  std::unique_ptr<Value> rightVal, Builder &b, SourcePos startPos,
                                                  SourcePos endPos) {
    auto leftTuple = dynamic_cast<Tuple*>(leftVal.get());
    auto rightTuple = dynamic_cast<Tuple*>(rightVal.get());

    if (leftTuple && rightTuple) {
        // if both sides are tuples, operate piece-wise
        auto leftSize = leftTuple->type()->types().size();
        auto rightSize = rightTuple->type()->types().size();

        std::vector<std::unique_ptr<Value>> resultVals;

        if (leftSize != rightSize) {
            throw CodegenError(
                "OOOOOOOOOOOOOOOOOOOOOOYYYYYY!!!!1! You're trying to " + MaximCommon::operatorType2Verb(type) + " " + std::to_string(leftSize) + " values to " + std::to_string(rightSize) + " ones!",
                startPos, endPos
            );
        }

        for (size_t i = 0; i < leftSize; i++) {
            auto leftTupleVal = leftTuple->atIndex(i, b, SourcePos(-1, -1), SourcePos(-1, -1));
            auto rightTupleVal = rightTuple->atIndex(i, b, SourcePos(-1, -1), SourcePos(-1, -1));
            auto op = alwaysGetOperator(type, leftTupleVal->type(), rightTupleVal->type(), startPos, endPos);
            resultVals.push_back(op->call(std::move(leftTupleVal), std::move(rightTuple)));
        }

        return Tuple::create(this, std::move(resultVals), b, startPos, endPos);
    } else if (leftTuple) {
        // if left is a tuple, splat right and operate piece-wise
        std::vector<std::unique_ptr<Value>> resultVals;

        auto leftSize = leftTuple->type()->types().size();
        for (size_t i = 0; i < leftSize; i++) {
            auto leftTupleVal = leftTuple->atIndex(i, b, SourcePos(-1, -1), SourcePos(-1, -1));
            auto op = alwaysGetOperator(type, leftTupleVal->type(), rightVal->type(), startPos, endPos);
            resultVals.push_back(op->call(std::move(leftTupleVal), rightVal->clone()));
        }

        return Tuple::create(this, std::move(resultVals), b, startPos, endPos);
    } else if (rightTuple) {
        // if right is a tuple, splat left and operate piece-wise
        std::vector<std::unique_ptr<Value>> resultVals;

        auto rightSize = rightTuple->type()->types().size();
        for (size_t i = 0; i < rightSize; i++) {
            auto rightTupleVal = rightTuple->atIndex(i, b, SourcePos(-1, -1), SourcePos(-1, -1));
            auto op = alwaysGetOperator(type, leftVal->type(), rightTupleVal->type(), startPos, endPos);
            resultVals.push_back(op->call(leftVal->clone(), std::move(rightTupleVal)));
        }

        return Tuple::create(this, std::move(resultVals), b, startPos, endPos);
    } else {
        // if neither are tuples, operate normally
        auto op = alwaysGetOperator(type, leftVal->type(), rightVal->type(), startPos, endPos);
        return op->call(std::move(leftVal), std::move(rightVal));
    }
}

Function* MaximContext::getFunction(std::string name, std::vector<Type *> types) {
    auto pos = functionMap.find(name);
    if (pos == functionMap.end() || pos->second.empty()) return nullptr;

    // try to find an overload that matches
    for (const auto &func : pos->second) {
        if (func->acceptsParameters(types)) return func.get();
    }

    // else return first one, let it display errors on validation later
    return pos->second[0].get();
}

std::unique_ptr<Value> MaximContext::callFunction(const std::string &name, std::vector<std::unique_ptr<Value>> values, Node *node, SourcePos startPos, SourcePos endPos) {
    std::vector<Type*> types;
    types.reserve(values.size());
    for (const auto &val : values) {
        types.push_back(val->type());
    }

    auto func = getFunction(name, types);
    if (!func) {
        throw CodegenError("WHAT IS THIS?\?!?! " + name + " is def not a valid function :(", startPos, endPos);
    }
    return func->call(node, std::move(values), startPos, endPos);
}

Converter* MaximContext::getConverter(MaximCommon::FormType destType) {
    auto pos = converterMap.find(destType);
    if (pos == converterMap.end()) return nullptr;
    return pos->second.get();
}

std::unique_ptr<Num> MaximContext::callConverter(MaximCommon::FormType destType, std::unique_ptr<Num> value) {
    auto con = getConverter(destType);
    assert(con);

    return con->call(std::move(value));
}

std::vector<std::unique_ptr<Function>>& MaximContext::getOrCreateFunctionList(std::string name) {
    auto pos = functionMap.find(name);
    if (pos == functionMap.end()) {
        return functionMap.emplace(name).first->second;
    } else {
        return pos->second;
    }
}

CodegenError MaximContext::typeAssertFailed(Type *expectedType, Type *receivedType, SourcePos startPos,
                                            SourcePos endPos) const {
    return CodegenError("Oyyyy m80, I need a " + expectedType->name() + " here, not this bad boi " + receivedType->name(), startPos, endPos);
}

Operator* MaximContext::alwaysGetOperator(MaximCommon::OperatorType type, Type *leftType, Type *rightType, SourcePos startPos, SourcePos endPos) {
    auto op = getOperator(type, leftType, rightType);
    if (!op) {
        throw CodegenError("WHAT IS THIS?\?!?! This operator doesn't work on these types of values.", startPos, endPos);
    }
    return op;
}
