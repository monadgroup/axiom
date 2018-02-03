#pragma once

#include <llvm/IR/IRBuilder.h>

#include "../SourcePos.h"

namespace MaximCodegen {

    class Function;
    class Value;

    class Context {
    public:
        enum class Type {
            FLOAT,
            INT4,
            INT8,
            INT32,

            FORM,
            NUM,
            MIDI,
            TUPLE
        };

        static constexpr size_t formParamCount = 2;

        Context();

        llvm::LLVMContext &llvm() { return _llvm; }

        llvm::Constant *getConstantInt(unsigned int numBits, uint64_t val, bool isSigned);

        llvm::Constant *getConstantFloat(float num);

        llvm::Value *getStructParamPtr(llvm::Value *ptr, llvm::Type *type, unsigned int param, llvm::IRBuilder<> &builder);

        llvm::Value *checkType(llvm::Value *val, llvm::Type *type, SourcePos start, SourcePos end);

        llvm::Value *checkType(llvm::Value *val, Type type, SourcePos start, SourcePos end);

        llvm::Type *getType(Type type);

        llvm::StructType *getStructType(Type type);

        Type getType(llvm::Type *type);

        std::string typeToString(llvm::Type *type);

        std::string typeToString(Type type);

        std::unique_ptr<Value> llToValue(bool isConst, llvm::Value *value);

    private:
        llvm::LLVMContext _llvm;
    };

}
