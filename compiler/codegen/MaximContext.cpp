#include "MaximContext.h"

#include <llvm/IR/Constants.h>

#include "Num.h"
#include "Midi.h"
#include "Tuple.h"
#include "Function.h"
#include "Operator.h"
#include "Converter.h"
#include "Node.h"

#include "functions/VectorIntrinsicFunction.h"
#include "functions/ScalarExternalFunction.h"
#include "functions/VectorIntrinsicFoldFunction.h"
#include "functions/ToRadFunction.h"
#include "functions/ToDegFunction.h"
#include "functions/ClampFunction.h"
#include "functions/PanFunction.h"
#include "functions/VectorShuffleFunction.h"
#include "functions/CombineFunction.h"
#include "functions/NoiseFunction.h"
#include "functions/ActiveFunction.h"
#include "functions/WithActiveFunction.h"
#include "functions/NextFunction.h"
#include "functions/DelayFunction.h"
#include "functions/AmplitudeFunction.h"
#include "functions/HoldFunction.h"
#include "functions/AccumFunction.h"
#include "functions/SineOscFunction.h"

#include "operators/NumFloatOperator.h"
#include "operators/NumIntrinsicOperator.h"
#include "operators/NumIntOperator.h"
#include "operators/NumComparisonOperator.h"
#include "operators/NumLogicalOperator.h"

#include "converters/BeatsConverter.h"
#include "converters/ControlConverter.h"
#include "converters/DbConverter.h"
#include "converters/FrequencyConverter.h"
#include "converters/LinearConverter.h"
#include "converters/SecondsConverter.h"

using namespace MaximCodegen;

// todo: remove dataLayout from MaximContext as it's only used in the runtime library
MaximContext::MaximContext(llvm::DataLayout dataLayout) : _dataLayout(dataLayout), _numType(this), _midiType(this) {
    /// REGISTER FUNCTIONS
    // functions that map directly to a built-in LLVM vector intrinsic
    registerFunction(VectorIntrinsicFunction::create(this, llvm::Intrinsic::ID::cos, "cos", 1));
    registerFunction(VectorIntrinsicFunction::create(this, llvm::Intrinsic::ID::sin, "sin", 1));
    registerFunction(VectorIntrinsicFunction::create(this, llvm::Intrinsic::ID::log, "log", 1));
    registerFunction(VectorIntrinsicFunction::create(this, llvm::Intrinsic::ID::log2, "log2", 1));
    registerFunction(VectorIntrinsicFunction::create(this, llvm::Intrinsic::ID::log10, "log10", 1));
    registerFunction(VectorIntrinsicFunction::create(this, llvm::Intrinsic::ID::sqrt, "sqrt", 1));
    registerFunction(VectorIntrinsicFunction::create(this, llvm::Intrinsic::ID::ceil, "ceil", 1));
    registerFunction(VectorIntrinsicFunction::create(this, llvm::Intrinsic::ID::floor, "floor", 1));
    registerFunction(VectorIntrinsicFunction::create(this, llvm::Intrinsic::ID::fabs, "abs", 1));

    // functions that map directly to an external scalar function
    registerFunction(ScalarExternalFunction::create(this, "tanf", "tan", 1));
    registerFunction(ScalarExternalFunction::create(this, "acosf", "acos", 1));
    registerFunction(ScalarExternalFunction::create(this, "asinf", "asin", 1));
    registerFunction(ScalarExternalFunction::create(this, "atanf", "atan", 1));
    registerFunction(ScalarExternalFunction::create(this, "atan2f", "atan2", 2));
    registerFunction(ScalarExternalFunction::create(this, "logbf", "logb", 1));
    registerFunction(ScalarExternalFunction::create(this, "hypotf", "hypot", 2));

    // other functions
    registerFunction(ToRadFunction::create(this));
    registerFunction(ToDegFunction::create(this));
    registerFunction(ClampFunction::create(this));
    registerFunction(PanFunction::create(this));
    registerFunction(VectorShuffleFunction::create(this, "left", {0, 0}));
    registerFunction(VectorShuffleFunction::create(this, "right", {1, 1}));
    registerFunction(VectorShuffleFunction::create(this, "swap", {1, 0}));
    registerFunction(CombineFunction::create(this));
    registerFunction(NoiseFunction::create(this));
    registerFunction(ActiveFunction::create(this));
    registerFunction(WithActiveFunction::create(this));
    registerFunction(SineOscFunction::create(this));
    registerFunction(NextFunction::create(this));
    registerFunction(DelayFunction::create(this));
    registerFunction(AmplitudeFunction::create(this));
    registerFunction(HoldFunction::create(this));
    registerFunction(AccumFunction::create(this));

    // hot paths for when only two parameters are provided to min/max
    registerFunction(VectorIntrinsicFunction::create(this, llvm::Intrinsic::ID::minnum, "min", 2));
    registerFunction(VectorIntrinsicFunction::create(this, llvm::Intrinsic::ID::maxnum, "max", 2));

    // variadic versions of min/max
    registerFunction(VectorIntrinsicFoldFunction::create(this, llvm::Intrinsic::ID::minnum, "min"));
    registerFunction(VectorIntrinsicFoldFunction::create(this, llvm::Intrinsic::ID::maxnum, "max"));

    /// REGISTER OPERATORS
    registerOperator(NumFloatOperator::create(this, MaximCommon::OperatorType::ADD, ActiveMode::ANY_INPUT, llvm::Instruction::BinaryOps::FAdd));
    registerOperator(NumFloatOperator::create(this, MaximCommon::OperatorType::SUBTRACT, ActiveMode::ANY_INPUT, llvm::Instruction::BinaryOps::FSub));
    registerOperator(NumFloatOperator::create(this, MaximCommon::OperatorType::MULTIPLY, ActiveMode::ALL_INPUTS, llvm::Instruction::BinaryOps::FMul));
    registerOperator(NumFloatOperator::create(this, MaximCommon::OperatorType::DIVIDE, ActiveMode::ALL_INPUTS, llvm::Instruction::BinaryOps::FDiv));
    registerOperator(NumFloatOperator::create(this, MaximCommon::OperatorType::MODULO, ActiveMode::ALL_INPUTS, llvm::Instruction::BinaryOps::FMul));
    registerOperator(NumIntrinsicOperator::create(this, MaximCommon::OperatorType::POWER, ActiveMode::FIRST_INPUT, llvm::Intrinsic::ID::pow));
    registerOperator(NumIntOperator::create(this, MaximCommon::OperatorType::BITWISE_AND, ActiveMode::ANY_INPUT, llvm::Instruction::BinaryOps::And, true));
    registerOperator(NumIntOperator::create(this, MaximCommon::OperatorType::BITWISE_OR, ActiveMode::ANY_INPUT, llvm::Instruction::BinaryOps::Or, true));
    registerOperator(NumIntOperator::create(this, MaximCommon::OperatorType::BITWISE_XOR, ActiveMode::ANY_INPUT, llvm::Instruction::BinaryOps::Xor, true));
    registerOperator(NumComparisonOperator::create(this, MaximCommon::OperatorType::LOGICAL_EQUAL, ActiveMode::ANY_INPUT, llvm::CmpInst::Predicate::FCMP_OEQ));
    registerOperator(NumComparisonOperator::create(this, MaximCommon::OperatorType::LOGICAL_NOT_EQUAL, ActiveMode::ANY_INPUT, llvm::CmpInst::Predicate::FCMP_ONE));
    registerOperator(NumComparisonOperator::create(this, MaximCommon::OperatorType::LOGICAL_GT, ActiveMode::ANY_INPUT, llvm::CmpInst::Predicate::FCMP_OGT));
    registerOperator(NumComparisonOperator::create(this, MaximCommon::OperatorType::LOGICAL_LT, ActiveMode::ANY_INPUT, llvm::CmpInst::Predicate::FCMP_OLT));
    registerOperator(NumComparisonOperator::create(this, MaximCommon::OperatorType::LOGICAL_GTE, ActiveMode::ANY_INPUT, llvm::CmpInst::Predicate::FCMP_OGE));
    registerOperator(NumComparisonOperator::create(this, MaximCommon::OperatorType::LOGICAL_LTE, ActiveMode::ANY_INPUT, llvm::CmpInst::Predicate::FCMP_OLE));
    registerOperator(NumLogicalOperator::create(this, MaximCommon::OperatorType::LOGICAL_AND, ActiveMode::ALL_INPUTS, llvm::Instruction::BinaryOps::And));
    registerOperator(NumLogicalOperator::create(this, MaximCommon::OperatorType::LOGICAL_OR, ActiveMode::ANY_INPUT, llvm::Instruction::BinaryOps::Or));

    /// REGISTER CONVERTERS
    registerConverter(BeatsConverter::create(this));
    registerConverter(ControlConverter::create(this));
    registerConverter(DbConverter::create(this));
    registerConverter(FrequencyConverter::create(this));
    registerConverter(LinearConverter::create(this));
    registerConverter(SecondsConverter::create(this));
}

llvm::Value* MaximContext::beatsPerSecond() const {
    return llvm::UndefValue::get(llvm::PointerType::get(numType()->vecType(), 0));
}

llvm::Type* MaximContext::voidPointerType() {
    return llvm::PointerType::get(llvm::Type::getInt1Ty(_llvm), 0);
}

void MaximContext::assertType(const Value *val, const Type *type) const {
    if (val->type() != type) {
        throw typeAssertFailed(type, val->type(), val->startPos, val->endPos);
    }
}

std::unique_ptr<Num> MaximContext::assertNum(std::unique_ptr<Value> val) {
    auto res = assertNum(val.get());
    val.release();
    return std::unique_ptr<Num>(res);
}

Num *MaximContext::assertNum(Value *val) {
    if (auto res = dynamic_cast<Num *>(val)) return res;
    throw typeAssertFailed(numType(), val->type(), val->startPos, val->endPos);
}

std::unique_ptr<Midi> MaximContext::assertMidi(std::unique_ptr<Value> val) {
    auto res = assertMidi(val.get());
    val.release();
    return std::unique_ptr<Midi>(res);
}

Midi *MaximContext::assertMidi(Value *val) {
    if (auto res = dynamic_cast<Midi *>(val)) return res;
    throw typeAssertFailed(midiType(), val->type(), val->startPos, val->endPos);
}

std::unique_ptr<Tuple> MaximContext::assertTuple(std::unique_ptr<Value> val, TupleType *type) {
    auto res = assertTuple(val.get(), type);
    val.release();
    return std::unique_ptr<Tuple>(res);
}

Tuple *MaximContext::assertTuple(Value *val, TupleType *type) {
    if (val->type() == type) {
        if (auto res = dynamic_cast<Tuple *>(val)) {
            return res;
        }
    }
    throw typeAssertFailed(type, val->type(), val->startPos, val->endPos);
}

TupleType *MaximContext::getTupleType(const std::vector<Type *> &types) {
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

ArrayType* MaximContext::getArrayType(Type *baseType) {
    auto arrayType = llvm::ArrayType::get(baseType->get(), ArrayType::arraySize);
    auto mapIndex = arrayTypeMap.find(arrayType);

    if (mapIndex == arrayTypeMap.end()) {
        return &arrayTypeMap.emplace(arrayType, ArrayType(this, baseType, arrayType)).first->second;
    } else {
        return &mapIndex->second;
    }
}

llvm::Constant *MaximContext::constFloat(float num) {
    return llvm::ConstantFP::get(llvm::Type::getFloatTy(_llvm), num);
}

llvm::Constant *MaximContext::constInt(unsigned int numBits, uint64_t val, bool isSigned) {
    return llvm::ConstantInt::get(llvm::Type::getIntNTy(_llvm, numBits), val, isSigned);
}

void MaximContext::registerOperator(std::unique_ptr<Operator> op) {
    OperatorKey key = {op->type(), op->leftType(), op->rightType()};
    operatorMap.emplace(key, std::move(op));
}

void MaximContext::registerFunction(std::unique_ptr<Function> func) {
    getOrCreateFunctionList(func->name()).push_back(std::move(func));
}

void MaximContext::registerConverter(std::unique_ptr<Converter> con) {
    converterMap.emplace(con->toType(), std::move(con));
}

Operator *MaximContext::getOperator(MaximCommon::OperatorType type, Type *leftType, Type *rightType) {
    OperatorKey key = {type, leftType, rightType};
    auto op = operatorMap.find(key);
    if (op == operatorMap.end()) return nullptr;
    return op->second.get();
}

std::unique_ptr<Value> MaximContext::callOperator(MaximCommon::OperatorType type, std::unique_ptr<Value> leftVal,
                                                  std::unique_ptr<Value> rightVal, Node *node, SourcePos startPos,
                                                  SourcePos endPos) {
    auto &b = node->builder();
    auto leftTuple = dynamic_cast<Tuple *>(leftVal.get());
    auto rightTuple = dynamic_cast<Tuple *>(rightVal.get());

    if (leftTuple && rightTuple) {
        // if both sides are tuples, operate piece-wise
        auto leftSize = leftTuple->type()->types().size();
        auto rightSize = rightTuple->type()->types().size();

        std::vector<std::unique_ptr<Value>> resultVals;

        if (leftSize != rightSize) {
            throw MaximCommon::CompileError(
                "OOOOOOOOOOOOOOOOOOOOOOYYYYYY!!!!1! You're trying to " + MaximCommon::operatorType2Verb(type) + " " +
                std::to_string(leftSize) + " values to " + std::to_string(rightSize) + " ones!",
                startPos, endPos
            );
        }

        for (size_t i = 0; i < leftSize; i++) {
            auto leftTupleVal = leftTuple->atIndex(i, b, SourcePos(-1, -1), SourcePos(-1, -1));
            auto rightTupleVal = rightTuple->atIndex(i, b, SourcePos(-1, -1), SourcePos(-1, -1));
            auto op = alwaysGetOperator(type, leftTupleVal->type(), rightTupleVal->type(), startPos, endPos);
            resultVals.push_back(op->call(node, std::move(leftTupleVal), std::move(rightTupleVal), startPos, endPos));
        }

        return Tuple::create(this, std::move(resultVals), b, startPos, endPos);
    } else if (leftTuple) {
        // if left is a tuple, splat right and operate piece-wise
        std::vector<std::unique_ptr<Value>> resultVals;

        auto leftSize = leftTuple->type()->types().size();
        for (size_t i = 0; i < leftSize; i++) {
            auto leftTupleVal = leftTuple->atIndex(i, b, SourcePos(-1, -1), SourcePos(-1, -1));
            auto op = alwaysGetOperator(type, leftTupleVal->type(), rightVal->type(), startPos, endPos);
            resultVals.push_back(op->call(node, std::move(leftTupleVal), rightVal->clone(), startPos, endPos));
        }

        return Tuple::create(this, std::move(resultVals), b, startPos, endPos);
    } else if (rightTuple) {
        // if right is a tuple, splat left and operate piece-wise
        std::vector<std::unique_ptr<Value>> resultVals;

        auto rightSize = rightTuple->type()->types().size();
        for (size_t i = 0; i < rightSize; i++) {
            auto rightTupleVal = rightTuple->atIndex(i, b, SourcePos(-1, -1), SourcePos(-1, -1));
            auto op = alwaysGetOperator(type, leftVal->type(), rightTupleVal->type(), startPos, endPos);
            resultVals.push_back(op->call(node, leftVal->clone(), std::move(rightTupleVal), startPos, endPos));
        }

        return Tuple::create(this, std::move(resultVals), b, startPos, endPos);
    } else {
        // if neither are tuples, operate normally
        auto op = alwaysGetOperator(type, leftVal->type(), rightVal->type(), startPos, endPos);
        return op->call(node, std::move(leftVal), std::move(rightVal), startPos, endPos);
    }
}

Function *MaximContext::getFunction(std::string name, std::vector<Type *> types) {
    auto pos = functionMap.find(name);
    if (pos == functionMap.end() || pos->second.empty()) return nullptr;

    // try to find an overload that matches
    for (const auto &func : pos->second) {
        if (func->acceptsParameters(types)) return func.get();
    }

    // else return first one, let it display errors on validation later
    return pos->second[0].get();
}

std::unique_ptr<Value>
MaximContext::callFunction(const std::string &name, std::vector<std::unique_ptr<Value>> values, Node *node,
                           SourcePos startPos, SourcePos endPos) {
    std::vector<Type *> types;
    types.reserve(values.size());
    for (const auto &val : values) {
        types.push_back(val->type());
    }

    auto func = getFunction(name, types);
    if (!func) {
        throw MaximCommon::CompileError("WHAT IS THIS?\?!?! " + name + " is def not a valid function :(", startPos, endPos);
    }
    return func->call(node, std::move(values), startPos, endPos);
}

Converter *MaximContext::getConverter(MaximCommon::FormType destType) {
    auto pos = converterMap.find(destType);
    if (pos == converterMap.end()) return nullptr;
    return pos->second.get();
}

std::unique_ptr<Num> MaximContext::callConverter(MaximCommon::FormType destType, std::unique_ptr<Num> value, Node *node,
                                                 SourcePos startPos, SourcePos endPos) {
    auto con = getConverter(destType);
    assert(con);

    return con->call(node, std::move(value), startPos, endPos);
}

void MaximContext::buildFunctions(llvm::Module *module) {
    for (const auto &pair : functionMap) {
        for (const auto &overload : pair.second) {
            overload->generate(module);
        }
    }
    for (const auto &pair : converterMap) {
        pair.second->generate(module);
    }
}

uint64_t MaximContext::secondsToSamples(float seconds) {
    return (uint64_t)(seconds * sampleRate);
}

llvm::Value* MaximContext::secondsToSamples(llvm::Value *seconds, Builder &b) {
    llvm::Constant *sampleRateConst = constFloat(sampleRate);
    llvm::Type *castType = llvm::Type::getInt64Ty(_llvm);
    if (seconds->getType()->isVectorTy()) {
        sampleRateConst = llvm::ConstantVector::getSplat(seconds->getType()->getVectorNumElements(), sampleRateConst);
        castType = llvm::VectorType::get(castType, seconds->getType()->getVectorNumElements());
    }

    auto floatResult = b.CreateBinOp(llvm::Instruction::BinaryOps::FMul, seconds, sampleRateConst, "samplerate.float");
    return b.CreateFPToUI(floatResult, castType, "samplerate.int64");
}

std::vector<std::unique_ptr<Function>> &MaximContext::getOrCreateFunctionList(std::string name) {
    auto pos = functionMap.find(name);
    if (pos == functionMap.end()) {
        return functionMap.emplace(name, std::vector<std::unique_ptr<Function>>{}).first->second;
    } else {
        return pos->second;
    }
}

MaximCommon::CompileError MaximContext::typeAssertFailed(const Type *expectedType, const Type *receivedType, SourcePos startPos,
                                            SourcePos endPos) const {
    return MaximCommon::CompileError(
        "Oyyyy m80, I need a " + expectedType->name() + " here, not this bad boi " + receivedType->name(), startPos,
        endPos);
}

Operator *
MaximContext::alwaysGetOperator(MaximCommon::OperatorType type, Type *leftType, Type *rightType, SourcePos startPos,
                                SourcePos endPos) {
    auto op = getOperator(type, leftType, rightType);
    if (!op) {
        throw MaximCommon::CompileError("WHAT IS THIS?\?!?! This operator doesn't work on these types of values.", startPos, endPos);
    }
    return op;
}
