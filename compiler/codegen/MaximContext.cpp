#include "MaximContext.h"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>

#include "Num.h"
#include "Midi.h"
#include "Tuple.h"
#include "TupleType.h"

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

}

void MaximContext::registerFunction(std::string name, std::unique_ptr<Function> func) {

}

void MaximContext::registerConverter(MaximCommon::FormType destType, std::unique_ptr<Converter> con) {

}

Operator* MaximContext::getOperator(MaximCommon::OperatorType type, llvm::Type *leftType, llvm::Type *rightType) {

}

std::unique_ptr<Value> MaximContext::callOperator(MaximCommon::OperatorType type, std::unique_ptr<Value> leftVal,
                                                  std::unique_ptr<Value> rightVal) {

}

Function* MaximContext::getFunction(std::string name, llvm::ArrayRef<llvm::Type *> types) {

}

std::unique_ptr<Value> MaximContext::callFunction(std::string name, llvm::ArrayRef<std::unique_ptr<Value>> values) {

}

Converter* MaximContext::getConverter(MaximCommon::FormType destType) {

}

std::unique_ptr<Num> MaximContext::callConverter(MaximCommon::FormType destType, std::unique_ptr<Num> value) {

}

CodegenError MaximContext::typeAssertFailed(Type *expectedType, Type *receivedType, SourcePos startPos,
                                            SourcePos endPos) const {
    return CodegenError("Oyyyy m80, I need a " + expectedType->name() + " here, not this bad boi " + receivedType->name(), startPos, endPos);
}
