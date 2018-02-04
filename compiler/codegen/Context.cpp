#include "Context.h"

#include <llvm/IR/InlineAsm.h>

#include "CodegenError.h"

#include "values/NumValue.h"
#include "values/MidiValue.h"
#include "values/TupleValue.h"

#include "FunctionDeclaration.h"
#include "Function.h"
#include "ControlDeclaration.h"

#include "../util.h"

using namespace MaximCodegen;

Context::Context() {
    llvm::Module builtinModule("builtins.llvm", llvm());

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

    addNumVecIntrinsic("cos", "llvm.cos.f32", 1, &builtinModule);
    addNumVecIntrinsic("sin", "llvm.sin.f32", 1, &builtinModule);
    addNumVecIntrinsic("tan", "tan", 1, &builtinModule);
    addNumVecIntrinsic("acos", "acos", 1, &builtinModule);
    addNumVecIntrinsic("asin", "asin", 1, &builtinModule);
    addNumVecIntrinsic("atan", "llvm.atan.f32", 1, &builtinModule);
    addNumVecIntrinsic("atan2", "atan2", 2, &builtinModule);
    addNumVecIntrinsic("hypot", "hypot", 2, &builtinModule);
    addNumVecIntrinsic("log", "llvm.log.f32", 1, &builtinModule);
    addNumVecIntrinsic("log2", "llvm.log2.f32", 1, &builtinModule);
    addNumVecIntrinsic("log10", "llvm.log10.f32", 1, &builtinModule);
    addNumVecIntrinsic("logb", "logb", 2, &builtinModule);
    addNumVecIntrinsic("sqrt", "llvm.sqrt.f32", 1, &builtinModule);
    addNumVecIntrinsic("ceil", "llvm.ceil.f32", 1, &builtinModule);
    addNumVecIntrinsic("floor", "llvm.floor.f32", 1, &builtinModule);
    addNumVecIntrinsic("round", "llvm.rint.f32", 1, &builtinModule);
    addNumVecIntrinsic("abs", "llvm.fabs.f32", 1, &builtinModule);

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
    auto type = getType(value->getType());
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

Function* Context::getFunctionDecl(const std::string &name) const {
    auto pair = functionDecls.find(name);
    if (pair == functionDecls.end()) return nullptr;
    return pair->second.get();
}

ControlDeclaration* Context::getControlDecl(MaximAst::ControlExpression::Type type) const {
    auto pair = controlDecls.find(type);
    assert(pair != controlDecls.end());
    return pair->second.get();
}

llvm::Function* Context::getVecIntrinsic(std::string name, size_t paramCount, llvm::Module *module) {
    auto numVec = llvm::VectorType::get(llvm::Type::getFloatTy(_llvm), 2);
    std::vector<llvm::Type*> items(paramCount, numVec);
    return llvm::Function::Create(
            llvm::FunctionType::get(numVec, items, false),
            llvm::Function::ExternalLinkage,
            name, module
    );
}

Function* Context::addFunc(std::string name, const FunctionDeclaration &decl, llvm::Module *module) {
    auto llFunc = llvm::Function::Create(decl.type(), llvm::Function::ExternalLinkage, name, module);
    auto newFunc = std::make_unique<Function>(decl, llFunc, this);
    auto newFuncPtr = newFunc.get();
    functionDecls.emplace(name, std::move(newFunc));
    return newFuncPtr;
}

Function* Context::addNumVecIntrinsic(std::string name, std::string intrinsicName, size_t paramCount, llvm::Module *module) {
    auto intrinsicFunc = getVecIntrinsic(std::move(intrinsicName), paramCount, module);

    std::vector<Parameter> funcParams(paramCount, Parameter(false, _numType));
    auto realFunc = addFunc(std::move(name), FunctionDeclaration(true, _numType, funcParams), module);
    auto cb = realFunc->codeBuilder();

    std::vector<llvm::Value*> paramValues;
    paramValues.reserve(paramCount);
    llvm::Value *firstFormPtr = nullptr;
    for (const auto &param : realFunc->llFunc()->args()) {
        auto paramX = AxiomUtil::strict_unique_cast<NumValue>(llToValue(false, &param));
        if (!firstFormPtr) firstFormPtr = paramX->formPtr(cb);
        paramValues.push_back(cb.CreateLoad(paramX->valuePtr(cb), "intrinsic_param"));
    }

    auto intrinsicResult = cb.CreateCall(intrinsicFunc, paramValues, "intrinsic_result");
    NumValue numResult(
            false, intrinsicResult,
            FormValue(firstFormPtr, this),
            this, realFunc
    );

    cb.CreateRet(numResult.value());
    return realFunc;
}
