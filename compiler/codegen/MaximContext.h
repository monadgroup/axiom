#pragma once

#include <llvm/IR/LLVMContext.h>
#include <unordered_map>

#include "../common/FormType.h"
#include "../common/OperatorType.h"
#include "../SourcePos.h"
#include "CodegenError.h"
#include "NumType.h"
#include "MidiType.h"
#include "Builder.h"

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

    class NumType;

    class MidiType;

    class TupleType;

    class Operator;

    class Function;

    class Converter;

    class Node;

    class MaximContext {
    public:
        MaximContext();

        llvm::LLVMContext &llvm() { return _llvm; }

        NumType *numType() const { return &_numType; }

        MidiType *midiType() const { return &_midiType; }

        void assertType(Value *val, Type *type) const;

        std::unique_ptr<Num> assertNum(std::unique_ptr<Value> val) const;

        Num *assertNum(Value *val) const;

        std::unique_ptr<Midi> assertMidi(std::unique_ptr<Value> val) const;

        Midi *assertMidi(Value *val) const;

        std::unique_ptr<Tuple> assertTuple(std::unique_ptr<Value> val, TupleType *type) const;

        Tuple *assertTuple(Value *val, TupleType *type) const;

        TupleType *getTupleType(const std::vector<Type *> &types);

        llvm::Constant *constFloat(float num) const;

        llvm::Constant *constInt(unsigned int numBits, uint64_t val, bool isSigned) const;

        void registerOperator(MaximCommon::OperatorType type, std::unique_ptr<Operator> op);

        void registerFunction(std::string name, std::unique_ptr<Function> func);

        void registerConverter(MaximCommon::FormType destType, std::unique_ptr<Converter> con);

        Operator *getOperator(MaximCommon::OperatorType type, Type *leftType, Type *rightType);

        std::unique_ptr<Value>
        callOperator(MaximCommon::OperatorType type, std::unique_ptr<Value> leftVal, std::unique_ptr<Value> rightVal,
                     Builder &b, SourcePos startPos, SourcePos endPos);

        Function *getFunction(std::string name, std::vector<Type *> types);

        std::unique_ptr<Value>
        callFunction(const std::string &name, std::vector<std::unique_ptr<Value>> values, Node *node,
                     SourcePos startPos, SourcePos endPos);

        Converter *getConverter(MaximCommon::FormType destType);

        std::unique_ptr<Num> callConverter(MaximCommon::FormType destType, std::unique_ptr<Num> value);

    private:
        llvm::LLVMContext _llvm;

        NumType _numType;
        MidiType _midiType;

        struct OperatorKey {
            MaximCommon::OperatorType type;
            Type *leftType;
            Type *rightType;
        };

        std::unordered_map<llvm::StructType *, TupleType> tupleTypeMap;
        std::unordered_map<OperatorKey, std::unique_ptr<Operator>> operatorMap;
        std::unordered_map<std::string, std::vector<std::unique_ptr<Function>>> functionMap;
        std::unordered_map<MaximCommon::FormType, std::unique_ptr<Converter>> converterMap;

        std::vector<std::unique_ptr<Function>> &getOrCreateFunctionList(std::string name);

        CodegenError
        typeAssertFailed(Type *expectedType, Type *receivedType, SourcePos startPos, SourcePos endPos) const;

        Operator *alwaysGetOperator(MaximCommon::OperatorType type, Type *leftType, Type *rightType, SourcePos startPos,
                                    SourcePos endPos);
    };

}
