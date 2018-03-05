#pragma once

#include "ComposableModuleClass.h"
#include "../SourcePos.h"

namespace MaximCodegen {

    class Type;

    class Value;

    class Parameter {
    public:
        Type *type;
        bool requireConst;
        bool optional;

        Parameter(Type *type, bool requireConst, bool optional);

        static std::unique_ptr<Parameter> create(Type *type, bool requireConst, bool optional);
    };

    class VarArg {
    public:
        explicit VarArg(MaximContext *context);

        std::unique_ptr<Value> atIndex(uint64_t index, Builder &b);

        virtual std::unique_ptr<Value> atIndex(llvm::Value *index, Builder &b) = 0;

        virtual llvm::Value *count(Builder &b) = 0;

    protected:
        MaximContext *context;
    };

    class ConstVarArg : public VarArg {
    public:
        std::vector<std::unique_ptr<Value>> vals;

        ConstVarArg(MaximContext *context, std::vector<std::unique_ptr<Value>> vals);

        std::unique_ptr<Value> atIndex(llvm::Value *index, Builder &b) override;

        std::unique_ptr<Value> atIndex(size_t index);

        llvm::Value *count(Builder &b) override;

        size_t count() const;
    };

    class Function : public ComposableModuleClass {
    public:
        Function(MaximContext *ctx, llvm::Module *module, const std::string &name, Type *returnType,
                 std::vector<Parameter> parameters, std::unique_ptr<Parameter> vararg, bool isPure = true);

        void generate();

        Type *returnType() const { return _returnType; }

        bool isPure() const { return _isPure; }

        const std::vector<Parameter> &parameters() const { return _parameters; }

        bool acceptsParameters(const std::vector<Type*> &types);

        std::unique_ptr<Value> call(ComposableModuleClassMethod *method, std::vector<std::unique_ptr<Value>> values, SourcePos startPos, SourcePos endPos);

        virtual std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params, std::unique_ptr<VarArg> vararg) = 0;

        virtual std::unique_ptr<Value>
        generateConst(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params, std::unique_ptr<ConstVarArg> vararg);

        virtual std::vector<std::unique_ptr<Value>> mapArguments(std::vector<std::unique_ptr<Value>> providedArgs);

        virtual void sampleArguments(ComposableModuleClassMethod *method, size_t index, const std::vector<std::unique_ptr<Value>> &args, const std::vector<std::unique_ptr<Value>> &varargs);

    private:
        class DynVarArg : public VarArg {
        public:
            llvm::Value *argStruct;
            Type *type;

            DynVarArg(MaximContext *context, llvm::Value *argStruct, Type *type);

            std::unique_ptr<Value> atIndex(llvm::Value *index, Builder &b) override;

            llvm::Value *count(Builder &b) override;
        };

        Type *_returnType;
        std::vector<Parameter> _parameters;
        std::unique_ptr<Parameter> _vararg;
        bool _isPure;

        llvm::Type *_vaType;
        size_t _vaIndex = 0;
        size_t _allArguments;
        size_t _minArguments;
        int _maxArguments;

        std::unique_ptr<ComposableModuleClassMethod> _callMethod;

        Parameter *getParameter(size_t index);

        bool validateCount(size_t passedCount, bool requireOptional);

        bool validateTypes(const std::vector<Type *> &types);

        void validateAndThrow(const std::vector<std::unique_ptr<Value>> &args, bool requireOptional, bool requireConst,
                              SourcePos startPos, SourcePos endPos);

        std::unique_ptr<Value>
        callConst(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &args,
                  std::vector<std::unique_ptr<Value>> varargs, SourcePos startPos, SourcePos endPos);

        std::unique_ptr<Value>
        callNonConst(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &allArgs,
                     const std::vector<std::unique_ptr<Value>> &args,
                     const std::vector<std::unique_ptr<Value>> &varargs, SourcePos startPos, SourcePos endPos);
    };

}
