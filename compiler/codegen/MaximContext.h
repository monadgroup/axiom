#pragma once

#include <llvm/IR/LLVMContext.h>

#include "../common/FormType.h"
#include "../common/OperatorType.h"
#include "ValueType.h"

namespace llvm {
    class Type;

    class VectorType;

    class StructType;

    class Constant;

    class Value;
}

namespace MaximCodegen {

    class Value;

    class Num;

    class Midi;

    class Tuple;

    class Operator;

    class Function;

    class Converter;

    class MaximContext {
    public:
        MaximContext();

        llvm::LLVMContext &llvm() { return _llvm; }

        llvm::VectorType *numVecType() const { return _numVecType; }

        llvm::Type *numFormType() const { return _numFormType; }

        llvm::Type *numActiveType() const { return _numActiveType; }

        llvm::StructType *numType() const { return _numType; }

        llvm::Type *midiEventType() const { return _midiEventType; }

        llvm::Type *midiChannelType() const { return _midiChannelType; }

        llvm::Type *midiNoteType() const { return _midiNoteType; }

        llvm::Type *midiParamType() const { return _midiParamType; }

        llvm::StructType *midiType() const { return _midiType; }

        llvm::Type *getType(ValueType type) const;

        void assertType(Value *val, ValueType type) const;

        std::unique_ptr<Value> createType(llvm::Value *val, ValueType type) const;

        std::unique_ptr<Num> assertNum(std::unique_ptr<Value> val) const;

        Num *assertNum(Value *val) const;

        std::unique_ptr<Midi> assertMidi(std::unique_ptr<Value> val) const;

        Midi *assertMidi(Value *val) const;

        std::unique_ptr<Tuple> assertTuple(std::unique_ptr<Value> val) const;

        Tuple *assertTuple(Value *val) const;

        llvm::Constant *constFloat(float num) const;

        llvm::Constant *constInt(size_t numBits, int64_t val, bool isSigned) const;

        Operator *getOperator(MaximCommon::OperatorType type, llvm::Type *leftType, llvm::Type *rightType);

        std::unique_ptr<Value>
        callOperator(MaximCommon::OperatorType type, std::unique_ptr<Value> leftVal, std::unique_ptr<Value> rightVal);

        Function *getFunction(std::string name, llvm::ArrayRef<llvm::Type *> types);

        std::unique_ptr<Value> callFunction(std::string name, llvm::ArrayRef<std::unique_ptr<Value>> values);

        Converter *getConverter(MaximCommon::FormType destType);

        std::unique_ptr<Num> callConverter(MaximCommon::FormType destType, std::unique_ptr<Num> value);

    private:
        llvm::LLVMContext _llvm;

        llvm::VectorType *_numVecType;
        llvm::Type *_numFormType;
        llvm::Type *_numActiveType;
        llvm::StructType *_numType;

        llvm::Type *_midiEventType;
        llvm::Type *_midiChannelType;
        llvm::Type *_midiNoteType;
        llvm::Type *_midiParamType;
        llvm::StructType *_midiType;
    };

}
