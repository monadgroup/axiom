#include "Function.h"

#include "Type.h"
#include "Value.h"
#include "MaximContext.h"
#include "ComposableModuleClassMethod.h"

using namespace MaximCodegen;

Function::Function(MaximContext *ctx, llvm::Module *module, const std::string &name, Type *returnType,
                   std::vector<Parameter> parameters, std::unique_ptr<Parameter> vararg, bool isPure)
    : ComposableModuleClass(ctx, module, name), _returnType(returnType), _parameters(std::move(parameters)),
      _vararg(std::move(vararg)), _isPure(isPure) {
    // calculate argument size constraints
    _allArguments = _parameters.size() + (_vararg ? 1 : 0);
    _minArguments = _allArguments;
    _maxArguments = _vararg ? -1 : (int) _parameters.size();


    std::vector<llvm::Type *> paramTypes;
    paramTypes.reserve(_allArguments);
    for (const auto &param : _parameters) {
        paramTypes.push_back(param.type->get());
        if (param.optional) _minArguments--;
    }

    if (_vararg) {
        _vaType = llvm::StructType::get(ctx->llvm(), {
            llvm::Type::getInt8Ty(ctx->llvm()),
            llvm::PointerType::get(_vararg->type->get(), 0)
        });
        _vaIndex = paramTypes.size();
        paramTypes.push_back(_vaType);
    }

    _callMethod = std::make_unique<ComposableModuleClassMethod>(this, "call", returnType->get(), paramTypes);

    // prettify arguments to pass into generate method
    std::vector<std::unique_ptr<Value>> genArgs;
    genArgs.reserve(_parameters.size());
    for (size_t i = 0; i < _parameters.size(); i++) {
        auto argVal = _callMethod->arg(i);
        auto argType = _parameters[i].type;
        genArgs.push_back(argType->createInstance(argVal, SourcePos(-1, -1), SourcePos(-1, -1)));
    }

    std::unique_ptr<VarArg> genVarArg;
    if (_vararg) genVarArg = std::make_unique<DynVarArg>(ctx, _callMethod->arg(_vaIndex), _vararg->type);
    generate(_callMethod.get(), genArgs, std::move(genVarArg));
}

bool Function::acceptsParameters(const std::vector<Type *> &types) {
    return validateCount(types.size(), false) && validateTypes(types);
}

std::unique_ptr<Value> Function::call(ComposableModuleClassMethod *method, std::vector<std::unique_ptr<Value>> values,
                                      llvm::Value *context, SourcePos startPos, SourcePos endPos) {
    // validate and map arguments - first validation can do without optional values, second can't
    validateAndThrow(values, false, true, startPos, endPos);
    auto mappedArgs = mapArguments(std::move(values));
    validateAndThrow(mappedArgs, true, true, startPos, endPos);

    // figure out if we can constant-fold this function
    auto computeConst = isPure();
    if (computeConst) {
        for (const auto &val : mappedArgs) {
            if (!llvm::isa<llvm::Constant>(val->get())) {
                computeConst = false;
                break;
            }
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
    if (computeConst) {
        return callConst(method, args, varargs, startPos, endPos);
    } else {
        return callNonConst(method, mappedArgs, args, varargs, startPos, endPos);
    }
}

std::unique_ptr<Value> Function::generateConst(ComposableModuleClassMethod *method,
                                               const std::vector<std::unique_ptr<Value>> &params,
                                               std::unique_ptr<ConstVarArg> vararg) {
    return generate(method, params, std::move(vararg));
}

std::vector<std::unique_ptr<Value>> Function::mapArguments(std::vector<std::unique_ptr<Value>> providedArgs) {
    return providedArgs;
}

Parameter* Function::getParameter(size_t index) {
    if (index < _parameters.size()) return &_parameters[index];
    assert(_vararg);
    return _vararg.get();
}

bool Function::validateCount(size_t passedCount, bool requireOptional) {
    return ((int) passedCount <= _maxArguments || _maxArguments < 0) &&
           passedCount >= (requireOptional ? _allArguments : _minArguments);
}

bool Function::validateTypes(const std::vector<Type *> &types) {
    for (size_t i = 0; i < types.size(); i++) {
        auto param = getParameter(i);
        if (param->type != types[i]) return false;
    }
    return true;
}

void Function::validateAndThrow(const std::vector<std::unique_ptr<Value>> &args, bool requireOptional,
                                bool requireConst, SourcePos startPos, SourcePos endPos) {
    if (!validateCount(args.size(), requireOptional)) {
        throw MaximCommon::CompileError("Eyy! That's the wrong number of arguments my dude.", startPos, endPos);
    }

    for (size_t i = 0; i < args.size(); i++) {
        auto providedArg = args[i].get();
        auto param = getParameter(i);

        ctx()->assertType(providedArg, param->type);
        if (requireConst && param->requireConst && !llvm::isa<llvm::Constant>(providedArg->get())) {
            throw MaximCommon::CompileError(
                "I constantly insist that constant values must be passed into constant parameters, and yet they constantly aren't constant.",
                providedArg->startPos, providedArg->endPos
            );
        }
    }
}

std::unique_ptr<Value> Function::callConst(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &args,
                                           std::vector<std::unique_ptr<Value>> varargs, SourcePos startPos, SourcePos endPos) {
    std::unique_ptr<ConstVarArg> vararg;
    if (_vararg) {
        assert(!varargs.empty());
        vararg = std::make_unique<ConstVarArg>(ctx(), std::move(varargs));
    }

    // evaluate function inline and let constant folding do the rest
    return generateConst(method, args, std::move(vararg))->withSource(startPos, endPos);
}

std::unique_ptr<Value> Function::callNonConst(ComposableModuleClassMethod *method,
                                              const std::vector<std::unique_ptr<Value>> &allArgs,
                                              const std::vector<std::unique_ptr<Value>> &args,
                                              const std::vector<std::unique_ptr<Value>> &varargs, SourcePos startPos,
                                              SourcePos endPos) {
    std::vector<llvm::Value *> values;
    values.reserve(args.size());
    for (const auto &arg : args) {
        values.push_back(arg->get());
    }

    // create vararg structure
    if (_vararg) {
        assert(!varargs.empty());
        auto vaType = _vararg->type->get();
        auto countVal = ctx()->constInt(8, varargs.size(), false);
        auto vaStruct = method->builder().CreateInsertValue(
            llvm::UndefValue::get(_vaType),
            countVal, {0}, "va.withsize"
        );
        auto vaArray = method->builder().CreateAlloca(vaType, countVal, "va.arr");
        for (size_t i = 0; i < varargs.size(); i++) {
            auto storePos = method->builder().CreateConstGEP1_64(vaArray, i, "va.arr.itemptr");
            method->builder().CreateStore(varargs[i]->get(), storePos);
        }
        values.push_back(method->builder().CreateInsertValue(vaStruct, vaArray, {1}, "va"));
    }

    auto entryIndex = method->moduleClass()->addEntry(this);
    auto result = method->callInto(entryIndex, values, _callMethod.get(), "callresult");
    return _returnType->createInstance(result, startPos, endPos);
}

Parameter::Parameter(Type *type, bool requireConst, bool optional)
    : type(type), requireConst(requireConst), optional(optional) {

}

std::unique_ptr<Parameter> Parameter::create(Type *type, bool requireConst, bool optional) {
    return std::make_unique<Parameter>(type, requireConst, optional);
}

VarArg::VarArg(MaximContext *context) : context(context) {

}

std::unique_ptr<Value> VarArg::atIndex(uint64_t index, Builder &b) {
    return atIndex(context->constInt(8, index, false), b);
}

ConstVarArg::ConstVarArg(MaximContext *context, std::vector<std::unique_ptr<Value>> vals)
    : VarArg(context), vals(std::move(vals)) {

}

std::unique_ptr<Value> ConstVarArg::atIndex(llvm::Value *index, Builder &b) {
    auto constVal = llvm::dyn_cast<llvm::ConstantInt>(index);
    assert(constVal);

    return vals[constVal->getZExtValue()]->clone();
}

std::unique_ptr<Value> ConstVarArg::atIndex(size_t index) {
    return vals[index]->clone();
}

llvm::Value* ConstVarArg::count(Builder &b) {
    return context->constInt(8, vals.size(), false);
}

size_t ConstVarArg::count() const {
    return vals.size();
}

Function::DynVarArg::DynVarArg(MaximContext *context, llvm::Value *argStruct, Type *type)
    : VarArg(context), argStruct(argStruct), type(type) {

}

std::unique_ptr<Value> Function::DynVarArg::atIndex(llvm::Value *index, Builder &b) {
    auto valPtr = b.CreateExtractValue(argStruct, {1}, "va.ptr");
    auto ptr = b.CreateGEP(type->get(), valPtr, {index}, "va.elementptr");
    return type->createInstance(b.CreateLoad(ptr, "va.element"), SourcePos(-1, -1), SourcePos(-1, -1));
}

llvm::Value* Function::DynVarArg::count(Builder &b) {
    return b.CreateExtractElement(argStruct, {(uint64_t) 0}, "va.count");
}
