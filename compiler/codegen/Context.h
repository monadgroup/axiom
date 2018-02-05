#pragma once

#include <llvm/IR/IRBuilder.h>
#include <unordered_map>
#include <string>
#include <llvm/IR/LegacyPassManager.h>

#include "../ast/ControlExpression.h"
#include "../SourcePos.h"
#include "FunctionDeclaration.h"

namespace MaximCodegen {

    class Value;

    class Function;

    class ControlDeclaration;

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

        llvm::Module *builtinModule() { return &_builtinModule; }

        llvm::Constant *getConstantInt(unsigned int numBits, uint64_t val, bool isSigned);

        llvm::Constant *getConstantFloat(float num);

        llvm::Value *getPtr(llvm::Value *ptr, unsigned int param, llvm::IRBuilder<> &builder);

        void checkType(llvm::Type *type, llvm::Type *expectedType, SourcePos start, SourcePos end);

        void checkType(llvm::Type *type, Type expectedType, SourcePos start, SourcePos end);

        void checkPtrType(llvm::Value *ptr, Type expectedType, SourcePos start, SourcePos end);

        llvm::Type *getType(Type type);

        llvm::StructType *getStructType(Type type);

        Type getType(llvm::Type *type);

        std::string typeToString(llvm::Type *type);

        std::string typeToString(Type type);

        std::unique_ptr<Value> llToValue(bool isConst, llvm::Value *value);

        Function *getFunction(const std::string &name) const;

        ControlDeclaration *getControlDecl(MaximAst::ControlExpression::Type type) const;

    private:
        llvm::LLVMContext _llvm;
        llvm::Module _builtinModule;

        std::unordered_map<std::string, std::unique_ptr<Function>> functionDecls;
        std::unordered_map<MaximAst::ControlExpression::Type, std::unique_ptr<ControlDeclaration>> controlDecls;

        llvm::StructType *_formType;
        llvm::StructType *_numType;
        llvm::StructType *_midiType;

        llvm::Function *getVecIntrinsic(llvm::Intrinsic::ID id, size_t paramCount, llvm::Module *module);
        Function *addFunc(std::string name, std::unique_ptr<FunctionDeclaration> decl, llvm::Module *module);
        Function *addNumVecIntrinsic(std::string name, llvm::Intrinsic::ID id, size_t paramCount, llvm::Module *module);
    };

}
