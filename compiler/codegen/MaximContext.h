#pragma once

#include <llvm/IR/LLVMContext.h>
#include <unordered_map>

#include "../common/FormType.h"
#include "../common/OperatorType.h"
#include "../common/CompileError.h"
#include "../common/ControlType.h"
#include "../SourcePos.h"
#include "NumType.h"
#include "MidiType.h"
#include "TupleType.h"
#include "ArrayType.h"
#include "Builder.h"

namespace MaximCodegen {
    struct OperatorKey {
        MaximCommon::OperatorType type;
        Type *leftType;
        Type *rightType;

        bool operator==(const MaximCodegen::OperatorKey &x) const {
            return type == x.type && leftType == x.leftType && rightType == x.rightType;
        }
    };
}

namespace std {
    template<>
    struct hash<MaximCodegen::OperatorKey> {
        size_t operator()(const MaximCodegen::OperatorKey &x) const {
            return (size_t) x.type ^ ((size_t) x.leftType) ^ ((size_t) x.rightType);
        }
    };
}

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

    class Array;

    class Operator;

    class Function;

    class Converter;

    class Control;

    class ModuleClassMethod;

    class ComposableModuleClassMethod;

    class MaximContext {
    public:
        explicit MaximContext(llvm::DataLayout dataLayout);

        uint64_t sampleRate = 44100;

        llvm::LLVMContext &llvm() { return _llvm; }

        llvm::DataLayout &dataLayout() { return _dataLayout; }

        // some LLVM globals
        llvm::Value *beatsPerSecond() const;

        NumType *numType() { return &_numType; }

        MidiType *midiType() { return &_midiType; }

        const NumType *numType() const { return &_numType; }

        const MidiType *midiType() const { return &_midiType; }

        llvm::StructType *vecScopeStorage() const { return _vecScopeStorage; }

        llvm::PointerType *voidPointerType();

        void assertType(const Value *val, const Type *type) const;

        std::unique_ptr<Num> assertNum(std::unique_ptr<Value> val);

        Num *assertNum(Value *val);

        std::unique_ptr<Midi> assertMidi(std::unique_ptr<Value> val);

        Midi *assertMidi(Value *val);

        std::unique_ptr<Tuple> assertTuple(std::unique_ptr<Value> val, TupleType *type);

        Tuple *assertTuple(Value *val, TupleType *type);

        std::unique_ptr<Array> assertArray(std::unique_ptr<Value> val, ArrayType *type);

        Array *assertArray(Value *val, ArrayType *type);

        TupleType *getTupleType(const std::vector<Type *> &types);

        ArrayType *getArrayType(Type *baseType);

        llvm::Constant *constFloat(float num);

        llvm::Type *floatVecTy();

        llvm::Constant *constFloatVec(float num);

        llvm::Constant *constFloatVec(float left, float right);

        llvm::Constant *constInt(unsigned int numBits, uint64_t val, bool isSigned);

        llvm::Constant *sizeOf(llvm::Type *type);

        void copyPtr(Builder &builder, llvm::Value *src, llvm::Value *dest);

        void clearPtr(Builder &builder, llvm::Value *src);

        void setLibModule(llvm::Module *libModule);

        const std::unordered_map<MaximCommon::FormType, std::unique_ptr<Converter>> &converters() const { return converterMap; }

        void registerOperator(std::unique_ptr<Operator> op);

        void registerFunction(std::unique_ptr<Function> func);

        void registerConverter(std::unique_ptr<Converter> con);

        void registerControl(std::unique_ptr<Control> con);

        Operator *getOperator(MaximCommon::OperatorType type, Type *leftType, Type *rightType);

        std::unique_ptr<Value>
        callOperator(MaximCommon::OperatorType type, std::unique_ptr<Value> leftVal, std::unique_ptr<Value> rightVal,
                     ModuleClassMethod *method, SourcePos startPos, SourcePos endPos);

        Function *getFunction(std::string name, std::vector<Type *> types);

        std::unique_ptr<Value>
        callFunction(const std::string &name, std::vector<std::unique_ptr<Value>> values,
                     ComposableModuleClassMethod *method,
                     SourcePos startPos, SourcePos endPos);

        Converter *getConverter(MaximCommon::FormType destType);

        std::unique_ptr<Num>
        callConverter(MaximCommon::FormType destType, std::unique_ptr<Num> value, ComposableModuleClassMethod *method,
                      SourcePos startPos, SourcePos endPos);

        Control *getControl(MaximCommon::ControlType type);

        uint64_t secondsToSamples(float seconds);

        llvm::Value *secondsToSamples(llvm::Value *seconds, Builder &b);

    private:
        llvm::LLVMContext _llvm;
        llvm::DataLayout _dataLayout;

        NumType _numType;
        MidiType _midiType;

        llvm::StructType *_vecScopeStorage;

        std::unique_ptr<Function> commonBiquadFunction;

        std::unordered_map<llvm::StructType *, TupleType> tupleTypeMap;
        std::unordered_map<llvm::Type *, ArrayType> arrayTypeMap;
        std::unordered_map<OperatorKey, std::unique_ptr<Operator>> operatorMap;
        std::unordered_map<std::string, std::vector<std::unique_ptr<Function>>> functionMap;
        std::unordered_map<MaximCommon::FormType, std::unique_ptr<Converter>> converterMap;
        std::unordered_map<MaximCommon::ControlType, std::unique_ptr<Control>> controlMap;

        std::vector<std::unique_ptr<Function>> &getOrCreateFunctionList(std::string name);

        MaximCommon::CompileError
        typeAssertFailed(const Type *expectedType, const Type *receivedType, SourcePos startPos,
                         SourcePos endPos) const;

        Operator *alwaysGetOperator(MaximCommon::OperatorType type, Type *leftType, Type *rightType, SourcePos startPos,
                                    SourcePos endPos);
    };

}
