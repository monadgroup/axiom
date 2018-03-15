#include "Function.h"

#include "Type.h"
#include "Value.h"
#include "MaximContext.h"
#include "ComposableModuleClassMethod.h"

using namespace MaximCodegen;

Function::Function(MaximContext *ctx, llvm::Module *module, const std::string &name, Type *returnType,
                   std::vector<Parameter> parameters, std::unique_ptr<Parameter> vararg, bool returnByRef)
    : ComposableModuleClass(ctx, module, name), _returnType(returnType), _parameters(std::move(parameters)),
      _vararg(std::move(vararg)), _returnByRef(returnByRef) {
    // calculate argument size constraints
    _allArguments = _parameters.size() + (_vararg ? 1 : 0);
    _minArguments = _allArguments;
    _maxArguments = _vararg ? -1 : (int) _parameters.size();

    std::vector<llvm::Type *> paramTypes;
    paramTypes.reserve(_allArguments);
    for (const auto &param : _parameters) {
        paramTypes.push_back(param.getType());
        if (param.optional) _minArguments--;
    }

    if (_vararg) {
        _vaType = llvm::StructType::get(ctx->llvm(), {
            llvm::Type::getInt8Ty(ctx->llvm()),
            llvm::PointerType::get(_vararg->getType(), 0)
        });
        _vaIndex = paramTypes.size();
        paramTypes.push_back(_vaType);
    }

    auto rt = returnType->get();
    // to prevent LLVM generating big FCA code blocks, we return a number with the size of the return type,
    // and bitcast it later when it's a pointer
    _warpedReturnType = llvm::IntegerType::get(ctx->llvm(), (unsigned int) ctx->dataLayout().getTypeAllocSize(rt));
    _callMethod = std::make_unique<ComposableModuleClassMethod>(this, "call", _warpedReturnType, paramTypes);
}

void Function::generate() {
    // prettify arguments to pass into generate method
    std::vector<std::unique_ptr<Value>> genArgs;
    genArgs.reserve(_parameters.size());
    for (size_t i = 0; i < _parameters.size(); i++) {
        genArgs.push_back(_parameters[i].type->createInstance(_callMethod->arg(i), SourcePos(-1, -1), SourcePos(-1, -1)));
    }

    std::unique_ptr<VarArg> genVarArg;
    if (_vararg) {
        genVarArg = std::make_unique<DynVarArg>(ctx(), _callMethod->arg(_vaIndex), _vararg->type);
    }
    auto result = generate(_callMethod.get(), genArgs, std::move(genVarArg));

    // todo: allow function to return a reference
    auto returnCast = _callMethod->builder().CreateBitCast(result->get(), llvm::PointerType::get(_warpedReturnType, 0), result->get()->getName() + ".warped");
    auto loadedResult = _callMethod->builder().CreateLoad(returnCast, "func.deref");
    _callMethod->builder().CreateRet(loadedResult);

    complete();
}

bool Function::acceptsParameters(const std::vector<Type *> &types) {
    return validateCount(types.size(), false) && validateTypes(types);
}

std::unique_ptr<Value> Function::call(ComposableModuleClassMethod *method, std::vector<std::unique_ptr<Value>> values,
                                      SourcePos startPos, SourcePos endPos) {
    // validate and map arguments - first validation can do without optional values, second can't
    validateAndThrow(values, false, startPos, endPos);
    auto mappedArgs = mapArguments(method, std::move(values));
    validateAndThrow(mappedArgs, true, startPos, endPos);

    std::vector<llvm::Value *> args;
    std::vector<Value *> baseArgs;
    std::vector<Value *> baseVarargs;
    for (size_t i = 0; i < _parameters.size(); i++) {
        auto arg = mappedArgs[i].get();
        baseArgs.push_back(arg);
        args.push_back(arg->get());
    }

    // create vararg structure
    if (_vararg) {
        auto vaCount = mappedArgs.size() - _parameters.size();
        auto vaType = _vararg->getType();
        auto countVal = ctx()->constInt(8, vaCount, false);
        auto vaStruct = method->builder().CreateInsertValue(
            llvm::UndefValue::get(_vaType),
            countVal, {0}, "va.withsize"
        );
        auto vaArray = method->builder().CreateAlloca(vaType, countVal, "va.arr");
        for (size_t i = 0; i < vaCount; i++) {
            auto vaArg = mappedArgs[_parameters.size() + i].get();
            baseVarargs.push_back(vaArg);

            auto storePos = method->builder().CreateConstGEP1_64(vaArray, i, "va.arr.itemptr");
            method->builder().CreateStore(vaArg->get(), storePos);
        }
        args.push_back(method->builder().CreateInsertValue(vaStruct, vaArray, {1}, "va"));
    }

    auto entryIndex = method->moduleClass()->addEntry(this);
    sampleArguments(method, entryIndex, baseArgs, baseVarargs);

    auto result = method->callInto(entryIndex, args, _callMethod.get(), "callresult");
    auto ptr = method->allocaBuilder().CreateAlloca(_warpedReturnType, nullptr, "callresult.ptr");
    method->builder().CreateStore(result, ptr);
    auto unwarpedPtr = method->builder().CreateBitCast(ptr, llvm::PointerType::get(_returnType->get(), 0), "unwarped");

    return _returnType->createInstance(unwarpedPtr, startPos, endPos);
}

std::vector<std::unique_ptr<Value>> Function::mapArguments(ComposableModuleClassMethod *method, std::vector<std::unique_ptr<Value>> providedArgs) {
    return providedArgs;
}

void Function::sampleArguments(ComposableModuleClassMethod *method, size_t index,
                               const std::vector<Value*> &args,
                               const std::vector<Value*> &varargs) {

}

Parameter *Function::getParameter(size_t index) {
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
                                SourcePos startPos, SourcePos endPos) {
    if (!validateCount(args.size(), requireOptional)) {
        throw MaximCommon::CompileError("Eyy! That's the wrong number of arguments my dude.", startPos, endPos);
    }

    for (size_t i = 0; i < args.size(); i++) {
        auto providedArg = args[i].get();
        auto param = getParameter(i);

        ctx()->assertType(providedArg, param->type);
    }
}

Parameter::Parameter(Type *type, bool optional)
    : type(type), optional(optional) {

}

std::unique_ptr<Parameter> Parameter::create(Type *type, bool optional) {
    return std::make_unique<Parameter>(type, optional);
}

llvm::Type* Parameter::getType() const {
    return llvm::PointerType::get(type->get(), 0);
}

VarArg::VarArg(MaximContext *context) : context(context) {

}

std::unique_ptr<Value> VarArg::atIndex(uint64_t index, Builder &b) {
    return atIndex(context->constInt(8, index, false), b);
}

Function::DynVarArg::DynVarArg(MaximContext *context, llvm::Value *argStruct, Type *type)
    : VarArg(context), argStruct(argStruct), type(type) {

}

std::unique_ptr<Value> Function::DynVarArg::atIndex(llvm::Value *index, Builder &b) {
    auto valPtr = b.CreateExtractValue(argStruct, 1, "va.ptr");
    auto elementPtr = b.CreateGEP(llvm::PointerType::get(type->get(), 0), valPtr, {index}, "va.elementptr");
    auto ptr = b.CreateLoad(elementPtr, "va.deref");
    return type->createInstance(ptr, SourcePos(-1, -1), SourcePos(-1, -1));
}

llvm::Value *Function::DynVarArg::count(Builder &b) {
    return b.CreateExtractValue(argStruct, (uint64_t) 0, "va.count");
}
