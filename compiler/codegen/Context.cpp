#include "Context.h"

#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Scalar.h>

#include <utility>

#include "CodegenError.h"

#include "values/NumValue.h"
#include "values/MidiValue.h"
#include "values/TupleValue.h"

#include "FunctionDeclaration.h"
#include "Function.h"
#include "ControlDeclaration.h"
#include "Control.h"

#include "../util.h"

using namespace MaximCodegen;

Context::Context() : _builtinModule("builtins.llvm", llvm()) {
    // generate built-in types
    _formType = llvm::StructType::create(_llvm, std::array<llvm::Type *, 2> {
            llvm::Type::getInt8Ty(_llvm), // type
            llvm::ArrayType::get(llvm::Type::getFloatTy(_llvm), formParamCount) // form params
    }, "form");

    _numType = llvm::StructType::create(_llvm, std::array<llvm::Type *, 2> {
            llvm::VectorType::get(llvm::Type::getFloatTy(_llvm), 2),
            _formType
    }, "num");

    _midiType = llvm::StructType::create(_llvm, std::array<llvm::Type *, 5> {
            llvm::Type::getIntNTy(_llvm, 4), // event type
            llvm::Type::getIntNTy(_llvm, 4), // channel
            llvm::Type::getInt8Ty(_llvm),    // note
            llvm::Type::getInt8Ty(_llvm),    // param
            llvm::Type::getInt32Ty(_llvm)    // time
    }, "midi");

    addNumVecIntrinsic("cos", llvm::Intrinsic::ID::cos, 1, &_builtinModule);
    addNumVecIntrinsic("sin", llvm::Intrinsic::ID::sin, 1, &_builtinModule);
    addNumScalarIntrinsic("tan", "tanf", 1, &_builtinModule);
    addNumScalarIntrinsic("acos", "acosf", 1, &_builtinModule);
    addNumScalarIntrinsic("asin", "asinf", 1, &_builtinModule);
    addNumScalarIntrinsic("atan", "atanf", 1, &_builtinModule);
    addNumScalarIntrinsic("atan2", "atan2f", 2, &_builtinModule);
    addNumScalarIntrinsic("hypot", "hypotf", 2, &_builtinModule);
    addNumVecIntrinsic("log", llvm::Intrinsic::ID::log, 1, &_builtinModule);
    addNumVecIntrinsic("log2", llvm::Intrinsic::ID::log2, 1, &_builtinModule);
    addNumVecIntrinsic("log10", llvm::Intrinsic::ID::log10, 1, &_builtinModule);
    addNumScalarIntrinsic("logb", "logbf", 2, &_builtinModule);
    addNumVecIntrinsic("sqrt", llvm::Intrinsic::ID::sqrt, 1, &_builtinModule);
    addNumVecIntrinsic("ceil", llvm::Intrinsic::ID::ceil, 1, &_builtinModule);
    addNumVecIntrinsic("floor", llvm::Intrinsic::ID::floor, 1, &_builtinModule);
    addNumVecIntrinsic("round", llvm::Intrinsic::ID::round, 1, &_builtinModule);
    addNumVecIntrinsic("abs", llvm::Intrinsic::ID::fabs, 1, &_builtinModule);

    // todo: min
    // todo: max
    // todo: clamp
    // todo: mix
    // todo: step
    // todo
}

llvm::Constant *Context::getConstantInt(unsigned int numBits, uint64_t val, bool isSigned) {
    return llvm::ConstantInt::get(_llvm, llvm::APInt(numBits, val, isSigned));
}

llvm::Constant *Context::getConstantFloat(float num) {
    return llvm::ConstantFP::get(_llvm, llvm::APFloat(num));
}

llvm::Value *Context::getPtr(llvm::Value *ptr, unsigned int param, llvm::IRBuilder<> &builder) {
    auto targetType = ptr->getType()->getPointerElementType();
    auto idxList = std::array<llvm::Value *, 2> {
            getConstantInt(32, 0, false),
            getConstantInt(32, param, false)
    };
    auto indexedType = llvm::GetElementPtrInst::getIndexedType(targetType, idxList);

    return builder.Insert(
            llvm::GetElementPtrInst::Create(ptr->getType()->getPointerElementType(), ptr, idxList),
            "ptr_" + typeToString(targetType) + "_" + std::to_string(param) + "_" + typeToString(indexedType)
    );
}

void Context::checkType(llvm::Type *type, llvm::Type *expectedType, SourcePos start, SourcePos end) {
    if (type != expectedType) {
        throw CodegenError(
                "Oyyyyy m80, I need a " + typeToString(expectedType) + " here, not this bad boi " +
                typeToString(type),
                start, end
        );
    }
}

void Context::checkType(llvm::Type *type, Type expectedType, SourcePos start, SourcePos end) {
    checkType(type, getType(expectedType), start, end);
}

void Context::checkPtrType(llvm::Value *ptr, Type expectedType, SourcePos start, SourcePos end) {
    checkType(ptr->getType()->getPointerElementType(), expectedType, start, end);
}

llvm::Type *Context::getType(Type type) {
    switch (type) {
        case Type::FLOAT:
            return llvm::Type::getFloatTy(_llvm);
        case Type::INT4:
            return llvm::Type::getIntNTy(_llvm, 4);
        case Type::INT8:
            return llvm::Type::getInt8Ty(_llvm);
        case Type::INT32:
            return llvm::Type::getInt32Ty(_llvm);
        default:
            return getStructType(type);
    }
}

llvm::StructType *Context::getStructType(Type type) {
    switch (type) {
        case Type::FORM:
            return _formType;
        case Type::NUM:
            return _numType;
        case Type::MIDI:
            return _midiType;
        default:
            assert(false);
    }

    throw;
}

Context::Type Context::getType(llvm::Type *type) {
    if (type == getType(Type::FLOAT)) return Type::FLOAT;
    if (type == getType(Type::INT4)) return Type::INT4;
    if (type == getType(Type::INT8)) return Type::INT8;
    if (type == getType(Type::INT32)) return Type::INT32;
    if (type == getType(Type::FORM)) return Type::FORM;
    if (type == getType(Type::NUM)) return Type::NUM;
    if (type == getType(Type::MIDI)) return Type::MIDI;
    else return Type::TUPLE;
}

std::string Context::typeToString(llvm::Type *type) {
    return typeToString(getType(type));
}

std::string Context::typeToString(Type type) {
    switch (type) {
        case Type::FLOAT:
            return "float";
        case Type::INT4:
            return "i4";
        case Type::INT8:
            return "i8";
        case Type::INT32:
            return "i32";
        case Type::FORM:
            return "form";
        case Type::NUM:
            return "num";
        case Type::MIDI:
            return "midi";
        case Type::TUPLE:
            return "tuple";
    }
    assert(false);
    throw;
}

std::unique_ptr<Value> Context::llToValue(bool isConst, llvm::Value *value) {
    assert(value->getType()->isPointerTy());
    auto type = getType(value->getType()->getPointerElementType());
    switch (type) {
        case Type::NUM:
            return std::make_unique<NumValue>(isConst, value, this);
        case Type::MIDI:
            return std::make_unique<MidiValue>(isConst, value, this);
        case Type::TUPLE:
            return std::make_unique<TupleValue>(isConst, value, this);
        default:
            assert(false);
    }

    throw;
}

Function* Context::getFunction(const std::string &name) const {
    auto pair = functionDecls.find(name);
    if (pair == functionDecls.end()) return nullptr;
    return pair->second.get();
}

ControlDeclaration* Context::getControlDecl(MaximAst::ControlExpression::Type type) const {
    auto pair = controlDecls.find(type);
    assert(pair != controlDecls.end());
    return pair->second.get();
}

llvm::Function* Context::getVecIntrinsic(llvm::Intrinsic::ID id, size_t paramCount, llvm::Module *module) {
    auto numVec = llvm::VectorType::get(llvm::Type::getFloatTy(_llvm), 2);
    std::vector<llvm::Type*> items(paramCount, numVec);
    return llvm::Intrinsic::getDeclaration(module, id, numVec);
}

llvm::Function* Context::getScalarIntrinsic(std::string name, size_t paramCount, llvm::Module *module) {
    auto floatTy = llvm::Type::getFloatTy(_llvm);
    std::vector<llvm::Type*> items(paramCount, floatTy);
    return llvm::Function::Create(
            llvm::FunctionType::get(floatTy, items, false),
            llvm::Function::ExternalLinkage,
            name, module
    );
}

Function* Context::addFunc(std::string name, std::unique_ptr<FunctionDeclaration> decl, llvm::Module *module) {
    auto newFunc = std::make_unique<Function>(std::move(decl), "maxim." + name, llvm::Function::InternalLinkage, module, this);
    auto newFuncPtr = newFunc.get();
    functionDecls.emplace(name, std::move(newFunc));
    return newFuncPtr;
}

Function* Context::addNumVecIntrinsic(std::string name, llvm::Intrinsic::ID id, size_t paramCount, llvm::Module *module) {
    auto intrinsicFunc = getVecIntrinsic(id, paramCount, module);

    std::vector<Parameter> funcParams(paramCount, Parameter(false, _numType));
    auto realFunc = addFunc(std::move(name), std::move(std::make_unique<FunctionDeclaration>(true, _numType, funcParams)), module);
    auto cb = realFunc->codeBuilder();

    std::vector<llvm::Value*> paramValues;
    paramValues.reserve(paramCount);
    llvm::Value *firstFormPtr = nullptr;
    for (auto &param : realFunc->llFunc()->args()) {
        auto paramDest = realFunc->initBuilder().CreateAlloca(_numType, nullptr, "param_temp");
        cb.CreateStore(&param, paramDest);
        auto paramX = std::make_unique<NumValue>(false, paramDest, this);
        if (!firstFormPtr) firstFormPtr = paramX->formPtr(cb);
        paramValues.push_back(cb.CreateLoad(paramX->valuePtr(cb), "intrinsic_param"));
    }

    auto intrinsicResult = cb.CreateCall(intrinsicFunc, paramValues, "intrinsic_result");
    NumValue numResult(
            false, intrinsicResult,
            FormValue(firstFormPtr, this),
            this, realFunc
    );

    cb.CreateRet(cb.CreateLoad(numResult.value(), "num_val_temp"));
    realFunc->initBuilder().CreateBr(realFunc->codeBlock());
    llvm::verifyFunction(*realFunc->llFunc());
    return realFunc;
}

Function* Context::addNumScalarIntrinsic(std::string name, std::string internalName, size_t paramCount,
                                         llvm::Module *module) {
    auto intrinsicFunc = getScalarIntrinsic(std::move(internalName), paramCount, module);

    std::vector<Parameter> funcParams(paramCount, Parameter(false, _numType));
    auto realFunc = addFunc(std::move(name), std::move(std::make_unique<FunctionDeclaration>(true, _numType, funcParams)), module);
    auto cb = realFunc->codeBuilder();

    llvm::Value *firstFormPtr = nullptr;
    std::vector<llvm::Value*> paramVecs;
    paramVecs.reserve(paramCount);
    for (auto &param : realFunc->llFunc()->args()) {
        auto paramDest = realFunc->initBuilder().CreateAlloca(_numType, nullptr, "param_temp");
        cb.CreateStore(&param, paramDest);
        auto paramX = std::make_unique<NumValue>(false, paramDest, this);
        if (!firstFormPtr) firstFormPtr = paramX->formPtr(cb);
        paramVecs.push_back(cb.CreateLoad(paramX->valuePtr(cb), "intrinsic_param"));
    }

    auto intrinsicResult = cb.CreateVectorSplat(2, getConstantFloat(0), "intrinsic_result");

    for (size_t i = 0; i < 2; i++) {
        std::vector<llvm::Value*> paramValues;
        paramValues.reserve(paramCount);
        for (auto &paramVec : paramVecs) {
            paramValues.push_back(cb.CreateExtractElement(paramVec, i, "intrinsic_single_param"));
        }

        auto intrinsicSingleResult = cb.CreateCall(intrinsicFunc, paramValues, "intrinsic_single_result");
        intrinsicResult = cb.CreateInsertElement(intrinsicResult, intrinsicSingleResult, i, "intrinsic_result");
    }

    NumValue numResult(
            false, intrinsicResult,
            FormValue(firstFormPtr, this),
            this, realFunc
    );

    cb.CreateRet(cb.CreateLoad(numResult.value(), "num_val_temp"));
    realFunc->initBuilder().CreateBr(realFunc->codeBlock());
    llvm::verifyFunction(*realFunc->llFunc());
    return realFunc;
}
