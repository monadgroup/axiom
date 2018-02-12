#pragma once

#include <memory>
#include <vector>
#include <llvm/ADT/ArrayRef.h>

#include "Builder.h"
#include "../SourcePos.h"

namespace llvm {
    class Type;

    class Constant;

    class Module;
}

namespace MaximCodegen {

    class Node;

    class Value;

    class FunctionCall;

    class MaximContext;

    class Type;

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

    class Function {
    public:
        Function(MaximContext *context, std::string name, Type *returnType, std::vector<Parameter> parameters,
                 std::unique_ptr<Parameter> vararg, llvm::Type *contextType, llvm::Module *module);

        std::string name() const { return _name; }

        Type *returnType() const { return _returnType; }

        bool isPure() const { return !_contextType; }

        std::vector<Parameter> const &parameters() const { return _parameters; }

        bool acceptsParameters(const std::vector<Type *> &types);

        std::unique_ptr<Value>
        call(Node *node, std::vector<std::unique_ptr<Value>> values, SourcePos startPos, SourcePos endPos);

    protected:
        llvm::Module *module() const { return _module; }

        llvm::Function *func() const { return _func; }

        MaximContext *context() const { return _context; }

        virtual std::unique_ptr<Value>
        generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg,
                 llvm::Value *funcContext) = 0;

        virtual std::unique_ptr<Value>
        generateConst(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg,
                      llvm::Value *funcContext);

        virtual std::vector<std::unique_ptr<Value>> mapArguments(std::vector<std::unique_ptr<Value>> providedArgs);

        virtual std::unique_ptr<FunctionCall> generateCall(std::vector<std::unique_ptr<Value>> args);

    private:
        class ConstVarArg : public VarArg {
        public:
            std::vector<std::unique_ptr<Value>> vals;

            ConstVarArg(MaximContext *context, std::vector<std::unique_ptr<Value>> vals);

            std::unique_ptr<Value> atIndex(llvm::Value *index, Builder &b) override;

            llvm::Value *count(Builder &b) override;
        };

        class DynVarArg : public VarArg {
        public:
            llvm::Value *argStruct;
            Type *type;

            DynVarArg(MaximContext *context, llvm::Value *argStruct, Type *type);

            std::unique_ptr<Value> atIndex(llvm::Value *index, Builder &b) override;

            llvm::Value *count(Builder &b) override;
        };

        MaximContext *_context;
        Type *_returnType;
        std::vector<Parameter> _parameters;
        std::unique_ptr<Parameter> _vararg;
        llvm::Type *_vaType;
        llvm::Type *_contextType;
        llvm::Function *_func;
        llvm::Module *_module;
        std::string _name;

        size_t _allArguments;
        size_t _minArguments;
        int _maxArguments;

        Parameter *getParameter(size_t index);

        bool validateCount(size_t passedCount, bool requireOptional);

        bool validateTypes(const std::vector<Type *> &types);

        void validateAndThrow(const std::vector<std::unique_ptr<Value>> &args, bool requireOptional, bool requireConst,
                              SourcePos startPos, SourcePos endPos);

        std::unique_ptr<Value>
        callConst(Node *node, std::vector<std::unique_ptr<Value>> args, std::vector<std::unique_ptr<Value>> varargs);

        std::unique_ptr<Value>
        callNonConst(Node *node, std::vector<std::unique_ptr<Value>> allArgs, std::vector<std::unique_ptr<Value>> args,
                     const std::vector<std::unique_ptr<Value>> &varargs, SourcePos startPos, SourcePos endPos);
    };

}
