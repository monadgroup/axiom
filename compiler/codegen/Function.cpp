#include "Function.h"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <sstream>

#include "MaximContext.h"
#include "FunctionCall.h"
#include "CodegenError.h"
#include "Value.h"
#include "Node.h"

using namespace MaximCodegen;

Function::Function(MaximContext *context, std::string name, Type *returnType, std::vector<Parameter> parameters,
                   std::unique_ptr<Parameter> vararg, llvm::Type *contextType, llvm::Module *module)
    : _context(context), _returnType(returnType), _parameters(std::move(parameters)),
      _vararg(std::move(vararg)), _contextType(contextType) {

    // calculate argument requirements
    _allArguments = _parameters.size() + (_vararg ? 1 : 0);
    _minArguments = _allArguments;
    _maxArguments = _vararg ? -1 : (int) _parameters.size();

    std::vector<llvm::Type *> paramTypes;
    paramTypes.reserve(_allArguments);

    for (const auto &param : _parameters) {
        paramTypes.push_back(param.type->get());
        if (param.optional) {
            _minArguments--;
        }
    }

    size_t vaIndex = 0, contextIndex = 0;

    // build vararg and context parameters
    if (_vararg) {
        _vaType = llvm::StructType::get(context->llvm(), {
            llvm::Type::getInt8Ty(context->llvm()),
            llvm::PointerType::get(_vararg->type->get(), 0)
        });
        vaIndex = paramTypes.size();
        paramTypes.push_back(_vaType);
    }
    if (_contextType) {
        contextIndex = paramTypes.size();
        paramTypes.push_back(llvm::PointerType::get(_contextType, 0));
    }

    // mangle name
    std::stringstream mangledName;
    mangledName << "maxim." << name;
    for (const auto &param : _parameters) {
        mangledName << "." << param.type->name();
    }
    if (_vararg) {
        mangledName << "_" << _vararg->type->name();
    }

    // create LLVM function
    auto funcType = llvm::FunctionType::get(_returnType->get(), paramTypes, false);
    _func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, mangledName.str(), module);

    // prettify arguments to pass into generate method
    std::vector<std::unique_ptr<Value>> genArgs;
    genArgs.reserve(_parameters.size());
    for (size_t i = 0; i < _parameters.size(); i++) {
        auto argVal = _func->arg_begin() + i;
        auto argType = _parameters[i].type;
        genArgs.push_back(argType->createInstance(argVal, SourcePos(-1, -1), SourcePos(-1, -1)));
    }

    std::unique_ptr<VarArg> genVarArg;
    if (_vararg) genVarArg = std::make_unique<DynVarArg>(_context, _func->arg_begin() + vaIndex, _vararg->type);

    llvm::Value *genContext = nullptr;
    if (_contextType) genContext = _func->arg_begin() + contextIndex;

    auto funcBlock = llvm::BasicBlock::Create(_context->llvm(), "entry", _func);
    Builder builder(funcBlock);
    auto res = generate(builder, genArgs, std::move(genVarArg), genContext);
    builder.CreateRet(res->get());
}

bool Function::acceptsParameters(const std::vector<llvm::Type *> &types) {
    return validateCount(types.size(), false) && validateTypes(types);
}

std::unique_ptr<Value>
Function::call(Node *node, std::vector<std::unique_ptr<Value>> values, SourcePos startPos,
               SourcePos endPos) {
    // validate and map arguments - first validation can do without optional values, second can't
    validateAndThrow(values, false, true, startPos, endPos);
    auto mappedArgs = mapArguments(std::move(values));
    validateAndThrow(mappedArgs, true, true, startPos, endPos);

    // figure out if we can constant-fold this function
    auto computeConst = isPure();
    for (const auto &val : mappedArgs) {
        if (!computeConst) break;
        if (!llvm::isa<llvm::Constant>(val->get())) {
            computeConst = false;
        }
    }

    // prepare argument vectors for call
    std::vector<std::unique_ptr<Value>> args;
    std::vector<std::unique_ptr<Value>> varargs;
    for (size_t i = 0; i < _parameters.size(); i++) {
        args.push_back(mappedArgs[i]->clone());
    }
    for (size_t i = args.size(); i < mappedArgs.size(); i++) {
        varargs.push_back(mappedArgs[i]->clone());
    }

    // either call inline with constant folding, or generate call
    if (computeConst) return callConst(node, std::move(args), std::move(varargs));
    else return callNonConst(node, std::move(mappedArgs), std::move(args), varargs, startPos, endPos);
}

std::unique_ptr<Value> Function::generateConst(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                               std::unique_ptr<VarArg> vararg, llvm::Value *context) {
    return generate(b, std::move(params), std::move(vararg), context);
}

std::vector<std::unique_ptr<Value>> Function::mapArguments(std::vector<std::unique_ptr<Value>> providedArgs) {
    return providedArgs;
}

std::unique_ptr<FunctionCall> Function::generateCall(std::vector<std::unique_ptr<Value>> args) {
    assert(false);
}

Parameter *Function::getParameter(size_t index) {
    if (index < _parameters.size()) return &_parameters[index];
    assert(_vararg);
    return _vararg.get();
}

bool Function::validateCount(size_t passedCount, bool requireOptional) {
    return (passedCount <= _maxArguments || _maxArguments < 0) &&
           passedCount >= (requireOptional ? _allArguments : _minArguments);
}

bool Function::validateTypes(const std::vector<Type*> &types) {
    for (size_t i = 0; i < types.size(); i++) {
        auto param = getParameter(i);
        if (param->type != types[i]) return false;
    }
    return true;
}

bool Function::validateArgs(const std::vector<std::unique_ptr<Value>> &args, bool requireConst) {
    for (size_t i = 0; i < args.size(); i++) {
        auto providedArg = args[i].get();
        auto param = getParameter(i);

        if (providedArg->type() != param->type) return false;
        if (requireConst && param->requireConst && !llvm::isa<llvm::Constant>(providedArg->get())) return false;
    }
    return true;
}

void Function::validateAndThrow(const std::vector<std::unique_ptr<Value>> &args, bool requireOptional,
                                bool requireConst, SourcePos startPos, SourcePos endPos) {
    if (!validateCount(args.size(), requireOptional)) {
        throw CodegenError("Eyy! That's the wrong number of arguments my dude.", startPos, endPos);
    }

    for (size_t i = 0; i < args.size(); i++) {
        auto providedArg = args[i].get();
        auto param = getParameter(i);

        _context->assertType(providedArg, param->type);
        if (requireConst && param->requireConst && !llvm::isa<llvm::Constant>(providedArg->get())) {
            throw CodegenError(
                "I constantly insist that constant values must be passed into constant parameters, and yet they constantly aren't constant.",
                providedArg->startPos, providedArg->endPos
            );
        }
    }
}

std::unique_ptr<Value> Function::callConst(Node *node, std::vector<std::unique_ptr<Value>> args,
                                           std::vector<std::unique_ptr<Value>> varargs) {
    std::unique_ptr<VarArg> vararg;
    if (_vararg) {
        assert(!varargs.empty());
        vararg = std::make_unique<ConstVarArg>(_context, std::move(varargs));
    }

    // evaluate function inline and let constant folding do the rest
    return generateConst(node->builder(), std::move(args), std::move(vararg), nullptr);
}

std::unique_ptr<Value> Function::callNonConst(Node *node,
                                              std::vector<std::unique_ptr<Value>> allArgs,
                                              std::vector<std::unique_ptr<Value>> args,
                                              const std::vector<std::unique_ptr<Value>> &varargs,
                                              SourcePos startPos, SourcePos endPos) {
    std::vector<llvm::Value *> values;
    values.reserve(args.size());
    for (const auto &arg : args) {
        values.push_back(arg->get());
    }

    // create vararg structure
    if (_vararg) {
        assert(!varargs.empty());
        auto vaType = _vararg->type->get();
        auto countVal = _context->constInt(8, varargs.size(), false);
        auto vaStruct = node->builder().CreateInsertValue(
            llvm::UndefValue::get(_vaType),
            countVal, {0}, "va.withsize"
        );
        auto vaArray = node->builder().CreateAlloca(vaType, countVal, "va.arr");
        for (size_t i = 0; i < varargs.size(); i++) {
            auto storePos = node->builder().CreateGEP(vaType, vaArray, {
                _context->constInt(32, i, false)
            }, "va.arr.itemptr");
            node->builder().CreateStore(varargs[i]->get(), storePos);
        }
        values.push_back(node->builder().CreateInsertValue(vaStruct, vaArray, {1}, "va"));
    }

    // create context in the parent node
    if (_contextType) {
        auto inst = generateCall(std::move(allArgs));
        auto contextPtr = node->addInstantiable(std::move(inst));
        assert(contextPtr->getType()->isPointerTy() && contextPtr->getType()->getPointerElementType() == _contextType);
        values.push_back(contextPtr);
    }

    auto result = node->builder().CreateCall(_func, values, "result");
    return _returnType->createInstance(result, startPos, endPos);
}

Parameter::Parameter(Type *type, bool requireConst, bool optional)
    : type(type), requireConst(requireConst), optional(optional) {}

VarArg::VarArg(MaximContext *context) : context(context) {}

std::unique_ptr<Value> VarArg::atIndex(uint64_t index, Builder &b) {
    return atIndex(context->constInt(8, index, false), b);
}

Function::ConstVarArg::ConstVarArg(MaximContext *context, std::vector<std::unique_ptr<Value>> vals)
    : VarArg(context), vals(std::move(vals)) {}

std::unique_ptr<Value> Function::ConstVarArg::atIndex(llvm::Value *index, Builder &b) {
    auto constVal = llvm::dyn_cast<llvm::ConstantInt>(index);
    assert(constVal);

    return vals[constVal->getZExtValue()]->clone();
}

llvm::Value *Function::ConstVarArg::count(Builder &b) {
    return context->constInt(8, vals.size(), false);
}

Function::DynVarArg::DynVarArg(MaximContext *context, llvm::Value *argStruct, Type *type)
    : VarArg(context), argStruct(argStruct), type(type) {}

std::unique_ptr<Value> Function::DynVarArg::atIndex(llvm::Value *index, Builder &b) {
    auto valPtr = b.CreateExtractValue(argStruct, {1}, "va.ptr");
    auto ptr = b.CreateGEP(type->get(), valPtr, {index}, "va.elementptr");
    return type->createInstance(b.CreateLoad(ptr, "va.element"), SourcePos(-1, -1), SourcePos(-1, -1));
}

llvm::Value *Function::DynVarArg::count(Builder &b) {
    return b.CreateExtractValue(argStruct, {0}, "va.count");
}
