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
    }, "struct.form");

    _numType = llvm::StructType::create(_llvm, std::array<llvm::Type *, 2> {
            llvm::VectorType::get(llvm::Type::getFloatTy(_llvm), 2),
            _formType
    }, "struct.num");

    _midiType = llvm::StructType::create(_llvm, std::array<llvm::Type *, 5> {
            llvm::Type::getIntNTy(_llvm, 4), // event type
            llvm::Type::getIntNTy(_llvm, 4), // channel
            llvm::Type::getInt8Ty(_llvm),    // note
            llvm::Type::getInt8Ty(_llvm),    // param
            llvm::Type::getInt32Ty(_llvm)    // time
    }, "struct.midi");

    /*_vaType = llvm::StructType::create(_llvm, std::array<llvm::Type *, 1> {
            llvm::Type::getInt8Ty(_llvm)
    }, "struct.va_list");*/
    /*_vaType = llvm::StructType::create(_llvm, {
            llvm::Type::getInt8Ty(_llvm), // number of arguments
            llvm::PointerType::get(llvm::ArrayType::get)
    }, "struct.va_list");*/

    addStandardLibrary();

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

llvm::Constant* Context::getConstantNum(float left, float right, llvm::Constant *form) {
    return llvm::ConstantStruct::get(_numType, std::array<llvm::Constant *, 2> {
            llvm::ConstantVector::get(std::array<llvm::Constant *, 2> {
                    getConstantFloat(left),
                    getConstantFloat(right)
            }),
            form
    });
}

llvm::Constant* Context::getConstantForm(MaximAst::Form::Type type, std::initializer_list<float> params) {
    std::vector<llvm::Constant *> paramConsts;
    paramConsts.reserve(params.size());
    for (const auto &p : params) {
        paramConsts.push_back(getConstantFloat(p));
    }

    return llvm::ConstantStruct::get(_formType, std::array<llvm::Constant *, 2> {
            getConstantInt(8, (unsigned int) type, false),
            llvm::ConstantArray::get(llvm::ArrayType::get(llvm::Type::getFloatTy(_llvm), formParamCount), paramConsts)
    });
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

llvm::Function* Context::getVecIntrinsic(llvm::Intrinsic::ID id, llvm::Module *module) {
    auto numVec = llvm::VectorType::get(llvm::Type::getFloatTy(_llvm), 2);
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

Function* Context::addNumVecIntrinsic(std::string name, llvm::Intrinsic::ID id, size_t paramCount, bool copyForm,
                                      llvm::Module *module) {
    auto intrinsicFunc = getVecIntrinsic(id, module);

    std::vector<Parameter> funcParams(paramCount, Parameter(false, _numType));
    auto realFunc = addFunc(std::move(name), std::move(std::make_unique<FunctionDeclaration>(true, _numType, funcParams)), module);
    auto cb = realFunc->codeBuilder();

    std::vector<llvm::Value*> paramValues;
    paramValues.reserve(paramCount);
    llvm::Value *firstFormPtr = nullptr;

    for (auto &param : realFunc->llFunc()->args()) {
        auto paramX = NumValue::fromRegister(false, &param, this, realFunc);
        if (!firstFormPtr) firstFormPtr = paramX->formPtr(cb);
        paramValues.push_back(cb.CreateLoad(paramX->valuePtr(cb), "intrinsic_param"));
    }

    auto intrinsicResult = cb.CreateCall(intrinsicFunc, paramValues, "intrinsic_result");

    FormValue val = copyForm ? FormValue(firstFormPtr, this)
                             : FormValue(MaximAst::Form::Type::LINEAR, {}, this, realFunc);

    NumValue numResult(false, intrinsicResult, val, this, realFunc);

    cb.CreateRet(cb.CreateLoad(numResult.value(), "num_val_temp"));
    realFunc->initBuilder().CreateBr(realFunc->codeBlock());
    llvm::verifyFunction(*realFunc->llFunc());
    return realFunc;
}

Function* Context::addNumScalarIntrinsic(std::string name, std::string internalName, size_t paramCount, bool copyForm,
                                         llvm::Module *module) {
    auto intrinsicFunc = getScalarIntrinsic(std::move(internalName), paramCount, module);

    std::vector<Parameter> funcParams(paramCount, Parameter(false, _numType));
    auto realFunc = addFunc(std::move(name), std::move(std::make_unique<FunctionDeclaration>(true, _numType, funcParams)), module);
    auto cb = realFunc->codeBuilder();

    llvm::Value *firstFormPtr = nullptr;
    std::vector<llvm::Value*> paramVecs;
    paramVecs.reserve(paramCount);

    for (auto &param : realFunc->llFunc()->args()) {
        auto paramX = NumValue::fromRegister(false, &param, this, realFunc);
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

    FormValue val = copyForm ? FormValue(firstFormPtr, this)
                             : FormValue(MaximAst::Form::Type::LINEAR, {}, this, realFunc);

    NumValue numResult(false, intrinsicResult, val, this, realFunc);

    cb.CreateRet(cb.CreateLoad(numResult.value(), "num_val_temp"));
    realFunc->initBuilder().CreateBr(realFunc->codeBlock());
    llvm::verifyFunction(*realFunc->llFunc());
    return realFunc;
}

Function* Context::addNumVecFoldIntrinsic(const std::string &name, llvm::Intrinsic::ID id, bool copyForm,
                                          llvm::Module *module) {
    auto vecType = llvm::VectorType::get(llvm::Type::getFloatTy(_llvm), 2);
    auto minIntrinsic = llvm::Intrinsic::getDeclaration(module, id, vecType);
    auto decl = std::make_unique<FunctionDeclaration>(true, _numType, std::vector<Parameter>{},
                                                      std::make_unique<Parameter>(false, _numType));
    auto func = addFunc(name, std::move(decl), module);

    auto cb = func->codeBuilder();
    auto vaArg = func->llFunc()->arg_begin();

    auto vaArgCount = cb.CreateExtractValue(vaArg, std::array<unsigned, 1> { 0 }, "arg_count");
    auto vaArrayPtr = cb.CreateExtractValue(vaArg, std::array<unsigned, 1> { 1 }, "arg_array");

    auto varCountType = llvm::Type::getInt8Ty(_llvm);
    auto incrementVal = func->initBuilder().CreateAlloca(varCountType, nullptr, "increment");
    cb.CreateStore(llvm::ConstantInt::get(varCountType, 0), incrementVal);

    auto accumVal = func->initBuilder().CreateAlloca(vecType, nullptr, "accum");
    auto firstReadPos = cb.Insert(
            llvm::GetElementPtrInst::Create(_numType, vaArrayPtr, {
                    getConstantInt(32, 0, false)
            }),
            "first_arg"
    );
    auto firstArg = std::make_unique<NumValue>(false, firstReadPos, this);
    cb.CreateStore(
            cb.CreateLoad(firstArg->valuePtr(cb), "first_arg_num"),
            accumVal
    );

    auto loopCheckBlock = llvm::BasicBlock::Create(_llvm, "loop_check", func->llFunc());
    auto loopContinueBlock = llvm::BasicBlock::Create(_llvm, "loop_continue", func->llFunc());
    auto loopFinishBlock = llvm::BasicBlock::Create(_llvm, "loop_finish", func->llFunc());

    cb.CreateBr(loopCheckBlock);

    llvm::IRBuilder<> loopCheckBuilder(loopCheckBlock);
    auto incrAdd = loopCheckBuilder.CreateAdd(
            loopCheckBuilder.CreateLoad(incrementVal, "increment_temp.load"),
            llvm::ConstantInt::get(varCountType, 1),
            "increment_temp.add"
    );
    loopCheckBuilder.CreateStore(incrAdd, incrementVal);
    auto incrResult = loopCheckBuilder.CreateICmpUGE(incrAdd, vaArgCount, "increment_compare");
    loopCheckBuilder.CreateCondBr(incrResult, loopFinishBlock, loopContinueBlock);

    func->codeBuilder().SetInsertPoint(loopContinueBlock);
    auto nextReadPos = func->codeBuilder().Insert(
            llvm::GetElementPtrInst::Create(_numType, vaArrayPtr, {incrAdd}), "next_arg"
    );
    auto nextArg = std::make_unique<NumValue>(false, nextReadPos, this);
    auto lastNum = func->codeBuilder().CreateLoad(accumVal, "last_num");
    auto nextNum = func->codeBuilder().CreateLoad(nextArg->valuePtr(func->codeBuilder()), "next_arg_num");
    auto resultVal = func->codeBuilder().CreateCall(minIntrinsic, std::array<llvm::Value*, 2> {
            lastNum,
            nextNum
    }, "intrinsic_result");
    func->codeBuilder().CreateStore(resultVal, accumVal);
    func->codeBuilder().CreateBr(loopCheckBlock);

    func->codeBuilder().SetInsertPoint(loopFinishBlock);

    FormValue newForm = copyForm ? FormValue(firstArg->formPtr(func->codeBuilder()), this)
                                 : FormValue(MaximAst::Form::Type::LINEAR, {}, this, func);

    auto finalAccum = func->codeBuilder().CreateLoad(accumVal, "accum_temp");
    auto newNum = std::make_unique<NumValue>(false, finalAccum, newForm, this, func);
    func->codeBuilder().CreateRet(func->codeBuilder().CreateLoad(newNum->value(), "accum_load"));

    func->initBuilder().CreateBr(func->codeBlock());

    return func;
}

void Context::addStandardLibrary() {
    // functions that map directly to libm
    addNumVecIntrinsic("cos", llvm::Intrinsic::ID::cos, 1, true, &_builtinModule);
    addNumVecIntrinsic("sin", llvm::Intrinsic::ID::sin, 1, true, &_builtinModule);
    addNumScalarIntrinsic("tan", "tanf", 1, true, &_builtinModule);
    addNumScalarIntrinsic("acos", "acosf", 1, true, &_builtinModule);
    addNumScalarIntrinsic("asin", "asinf", 1, true, &_builtinModule);
    addNumScalarIntrinsic("atan", "atanf", 1, true, &_builtinModule);
    addNumScalarIntrinsic("atan2", "atan2f", 2, false, &_builtinModule);
    addNumScalarIntrinsic("hypot", "hypotf", 2, false, &_builtinModule);
    addNumVecIntrinsic("log", llvm::Intrinsic::ID::log, 1, true, &_builtinModule);
    addNumVecIntrinsic("log2", llvm::Intrinsic::ID::log2, 1, true, &_builtinModule);
    addNumVecIntrinsic("log10", llvm::Intrinsic::ID::log10, 1, true, &_builtinModule);
    addNumScalarIntrinsic("logb", "logbf", 2, false, &_builtinModule);
    addNumVecIntrinsic("sqrt", llvm::Intrinsic::ID::sqrt, 1, true, &_builtinModule);
    addNumVecIntrinsic("ceil", llvm::Intrinsic::ID::ceil, 1, true, &_builtinModule);
    addNumVecIntrinsic("floor", llvm::Intrinsic::ID::floor, 1, true, &_builtinModule);
    addNumVecIntrinsic("round", llvm::Intrinsic::ID::round, 1, true, &_builtinModule);
    addNumVecIntrinsic("abs", llvm::Intrinsic::ID::fabs, 1, true, &_builtinModule);
    addNumVecFoldIntrinsic("min", llvm::Intrinsic::ID::minnum, false, &_builtinModule);
    addNumVecFoldIntrinsic("max", llvm::Intrinsic::ID::maxnum, false, &_builtinModule);

    auto vecType = llvm::VectorType::get(llvm::Type::getFloatTy(_llvm), 2);

    // pure num clamp(num x, num min, num max)
    {
        auto defaultMin = getConstantNum(-1, -1, getConstantForm(MaximAst::Form::Type::LINEAR, {0, 1}));
        auto defaultMax = getConstantNum(1, 1, getConstantForm(MaximAst::Form::Type::LINEAR, {0, 1}));
        auto minIntrinsic = llvm::Intrinsic::getDeclaration(&_builtinModule, llvm::Intrinsic::ID::minnum, vecType);
        auto maxIntrinsic = llvm::Intrinsic::getDeclaration(&_builtinModule, llvm::Intrinsic::ID::maxnum, vecType);

        auto func = addFunc("clamp", std::make_unique<FunctionDeclaration>(
                true, _numType, std::vector<Parameter> {
                        Parameter(false, _numType),
                        Parameter(false, _numType, defaultMin),
                        Parameter(false, _numType, defaultMax)
                }
        ), &_builtinModule);

        auto numVal = NumValue::fromRegister(false, func->llFunc()->arg_begin() + 0, this, func);
        auto minVal = NumValue::fromRegister(false, func->llFunc()->arg_begin() + 1, this, func);
        auto maxVal = NumValue::fromRegister(false, func->llFunc()->arg_begin() + 2, this, func);

        auto cb = func->codeBuilder();
        auto biggerValue = cb.CreateCall(maxIntrinsic, std::array<llvm::Value*, 2> {
                cb.CreateLoad(numVal->valuePtr(cb), "load_num"),
                cb.CreateLoad(minVal->valuePtr(cb), "load_min")
        }, "bigger_val");
        auto smallerValue = cb.CreateCall(minIntrinsic, std::array<llvm::Value*, 2> {
                biggerValue,
                cb.CreateLoad(maxVal->valuePtr(cb), "load_max")
        }, "smaller_val");

        auto newNum = std::make_unique<NumValue>(
                false, smallerValue, FormValue(MaximAst::Form::Type::LINEAR, {}, this, func), this, func
        );
        cb.CreateRet(cb.CreateLoad(newNum->value(), "value_temp"));

        func->initBuilder().CreateBr(func->codeBlock());
    }

    // pure num mix(num a, num b, num v)
    {
        auto func = addFunc("mix", std::make_unique<FunctionDeclaration>(
                true, _numType, std::vector<Parameter> {
                        Parameter(false, _numType),
                        Parameter(false, _numType),
                        Parameter(false, _numType)
                }
        ), &_builtinModule);

        auto aVal = NumValue::fromRegister(false, func->llFunc()->arg_begin() + 0, this, func);
        auto bVal = NumValue::fromRegister(false, func->llFunc()->arg_begin() + 1, this, func);
        auto vVal = NumValue::fromRegister(false, func->llFunc()->arg_begin() + 2, this, func);

        auto cb = func->codeBuilder();
        auto aNum = cb.CreateLoad(aVal->valuePtr(cb), "a_num");
        auto abDiff = cb.CreateFSub(
                cb.CreateLoad(bVal->valuePtr(cb), "b_num"),
                aNum,
                "ab_diff"
        );
        auto abMul = cb.CreateFMul(
                abDiff,
                cb.CreateLoad(vVal->valuePtr(cb), "v_num"),
                "ab_mul"
        );
        auto resultNum = cb.CreateFAdd(
                aNum,
                abMul,
                "am_add"
        );
        auto newNum = std::make_unique<NumValue>(
                false, resultNum, FormValue(MaximAst::Form::Type::LINEAR, {}, this, func), this, func
        );
        cb.CreateRet(cb.CreateLoad(newNum->value(), "value_temp"));

        func->initBuilder().CreateBr(func->codeBlock());
    }

    // pure num pan(num x, num pan)
    /*{
        auto func = addFunc("pan", std::make_unique<FunctionDeclaration>(
                true, _numType, std::vector<Parameter> {
                        Parameter(false, _numType),
                        Parameter(false, _numType)
                }
        ), &_builtinModule);

        auto xVal = NumValue::fromRegister(false, func->llFunc()->arg_begin() + 1, this, func);
        auto panVal = NumValue::fromRegister(false, func->llFunc()->arg_begin() + 2, this, func);
    }*/

    // pure num left(num x)
    {
        auto func = addFunc("left", std::make_unique<FunctionDeclaration>(
                true, _numType, std::vector<Parameter> {Parameter(false, _numType)}
        ), &_builtinModule);

        auto xVal = NumValue::fromRegister(false, func->llFunc()->arg_begin(), this, func);
        auto cb = func->codeBuilder();
        auto xValNum = cb.CreateLoad(xVal->valuePtr(cb), "num_temp");
        auto leftVec = cb.CreateVectorSplat(
                2,
                cb.CreateExtractElement(xValNum, (uint64_t) 0, "num_left"),
                "num_left_splat"
        );

        auto numVal = std::make_unique<NumValue>(false, leftVec, FormValue(xVal->formPtr(cb), this), this, func);
        cb.CreateRet(cb.CreateLoad(numVal->value(), "num_result"));

        func->initBuilder().CreateBr(func->codeBlock());
    }

    // pure num right(num x)
    {
        auto func = addFunc("right", std::make_unique<FunctionDeclaration>(
                true, _numType, std::vector<Parameter> {Parameter(false, _numType)}
        ), &_builtinModule);

        auto xVal = NumValue::fromRegister(false, func->llFunc()->arg_begin(), this, func);
        auto cb = func->codeBuilder();
        auto xValNum = cb.CreateLoad(xVal->valuePtr(cb), "num_temp");
        auto leftVec = cb.CreateVectorSplat(
                2,
                cb.CreateExtractElement(xValNum, (uint64_t) 1, "num_right"),
                "num_right_splat"
        );

        auto numVal = std::make_unique<NumValue>(false, leftVec, FormValue(xVal->formPtr(cb), this), this, func);
        cb.CreateRet(cb.CreateLoad(numVal->value(), "num_result"));

        func->initBuilder().CreateBr(func->codeBlock());
    }

    // pure num combine(num left, num right)
    {
        auto func = addFunc("combine", std::make_unique<FunctionDeclaration>(
                true, _numType, std::vector<Parameter> {Parameter(false, _numType), Parameter(false, _numType)}
        ), &_builtinModule);

        auto leftVal = NumValue::fromRegister(false, func->llFunc()->arg_begin(), this, func);
        auto rightVal = NumValue::fromRegister(false, func->llFunc()->arg_begin() + 1, this, func);
        auto cb = func->codeBuilder();
        auto leftVec = cb.CreateLoad(leftVal->valuePtr(cb), "left_temp");
        auto rightVec = cb.CreateLoad(rightVal->valuePtr(cb), "right_temp");
        auto shuffledVec = cb.CreateShuffleVector(
                leftVec, rightVec,
                std::array<uint32_t, 2> { 0, 3 },
                "num_shuffled"
        );

        auto numVal = std::make_unique<NumValue>(
                false, shuffledVec, FormValue(MaximAst::Form::Type::LINEAR, {}, this, func),
                this, func
        );
        cb.CreateRet(cb.CreateLoad(numVal->value(), "num_result"));

        func->initBuilder().CreateBr(func->codeBlock());
    }

    // pure num swap(num x)
    {
        auto func = addFunc("swap", std::make_unique<FunctionDeclaration>(
                true, _numType, std::vector<Parameter> {Parameter(false, _numType)}
        ), &_builtinModule);

        auto xVal = NumValue::fromRegister(false, func->llFunc()->arg_begin(), this, func);
        auto cb = func->codeBuilder();
        auto xVec = cb.CreateLoad(xVal->valuePtr(cb), "x_temp");
        auto shuffledVec = cb.CreateShuffleVector(
                xVec, xVec,
                std::array<uint32_t, 2> { 1, 0 },
                "x_swapped"
        );

        auto numVal = std::make_unique<NumValue>(false, shuffledVec, FormValue(xVal->formPtr(cb), this), this, func);
        cb.CreateRet(cb.CreateLoad(numVal->value(), "num_result"));

        func->initBuilder().CreateBr(func->codeBlock());
    }

    // pure num sequence(num n, num ...x)
    {
        auto func = addFunc("sequence", std::make_unique<FunctionDeclaration>(
                true, _numType, std::vector<Parameter> {Parameter(false, _numType)},
                std::make_unique<Parameter>(false, _numType)
        ), &_builtinModule);

        auto cb = func->codeBuilder();
        auto indexVal = NumValue::fromRegister(false, func->llFunc()->arg_begin(), this, func);

        auto vaArg = func->llFunc()->arg_begin() + 1;
        auto vaArgCount = cb.CreateExtractValue(vaArg, std::array<unsigned, 1> { 0 }, "arg_count");
        auto vaArrayPtr = cb.CreateExtractValue(vaArg, std::array<unsigned, 1> { 1 }, "arg_array");

        auto argCountSplat = cb.CreateVectorSplat(2, vaArgCount, "arg_count_splat");
        auto countTy = vaArgCount->getType();
        auto indexNum = cb.CreateLoad(indexVal->valuePtr(cb), "index_temp");
        auto baseIndex = cb.CreateFPToSI(
                cb.CreateFRem(
                        indexNum,
                        cb.CreateUIToFP(
                                argCountSplat,
                                llvm::VectorType::get(llvm::Type::getFloatTy(_llvm), 2)
                        )
                ),
                llvm::VectorType::get(countTy, 2),
                "base_index"
        );
        auto incrVal = llvm::ConstantInt::get(countTy, 1);
        auto nextIndex = cb.CreateURem(
                cb.CreateAdd(
                    baseIndex,
                    llvm::ConstantVector::get({ incrVal, incrVal }),
                    "next_index_raw"
                ),
                argCountSplat
        );

        auto baseLeftPtr = cb.Insert(
                llvm::GetElementPtrInst::Create(_numType, vaArrayPtr, {
                        cb.CreateExtractElement(baseIndex, (uint64_t) 0, "base_left_index")
                }),
                "base_left_ptr"
        );
        auto baseLeftNum = std::make_unique<NumValue>(false, baseLeftPtr, this);
        auto baseRightPtr = cb.Insert(
                llvm::GetElementPtrInst::Create(_numType, vaArrayPtr, {
                        cb.CreateExtractElement(baseIndex, (uint64_t) 1, "base_right_index")
                }),
                "base_right_ptr"
        );
        auto baseRightNum = std::make_unique<NumValue>(false, baseRightPtr, this);

        auto baseValLeft = cb.CreateLoad(baseLeftNum->valuePtr(cb), "base_left_tmp");
        auto baseValRight = cb.CreateLoad(baseRightNum->valuePtr(cb), "base_right_tmp");
        auto baseVal = cb.CreateShuffleVector(baseValLeft, baseValRight, {0, 3}, "base_val");

        auto nextLeftPtr = cb.Insert(
                llvm::GetElementPtrInst::Create(_numType, vaArrayPtr, {
                        cb.CreateExtractElement(nextIndex, (uint64_t) 0, "next_left_index")
                }),
                "next_left_ptr"
        );
        auto nextLeftNum = std::make_unique<NumValue>(false, nextLeftPtr, this);
        auto nextRightPtr = cb.Insert(
                llvm::GetElementPtrInst::Create(_numType, vaArrayPtr, {
                        cb.CreateExtractElement(nextIndex, (uint64_t) 1, "next_right_index")
                }),
                "next_right_ptr"
        );
        auto nextRightNum = std::make_unique<NumValue>(false, nextRightPtr, this);

        auto nextValLeft = cb.CreateLoad(nextLeftNum->valuePtr(cb), "next_left_tmp");
        auto nextValRight = cb.CreateLoad(nextRightNum->valuePtr(cb), "next_right_tmp");
        auto nextVal = cb.CreateShuffleVector(nextValLeft, nextValRight, {0, 3}, "next_val");

        auto indexRem = cb.CreateFRem(
                indexNum,
                llvm::ConstantVector::get({ getConstantFloat(1), getConstantFloat(1) }),
                "index_rem"
        );
        auto valDiff = cb.CreateFSub(nextVal, baseVal, "val_diff");
        auto valMul = cb.CreateFMul(valDiff, indexRem, "val_mul");
        auto valAdd = cb.CreateFAdd(baseVal, valMul, "val_add");

        auto newNum = std::make_unique<NumValue>(
                false, valAdd, FormValue(MaximAst::Form::Type::LINEAR, {}, this, func),
                this, func
        );
        cb.CreateRet(cb.CreateLoad(newNum->value(), "num_load"));

        func->initBuilder().CreateBr(func->codeBlock());
    }

    // num noise(min=-1, max=1)
    {
        auto randIntrinsic = llvm::Function::Create(
                llvm::FunctionType::get(llvm::Type::getInt32Ty(_llvm), {}, false),
                llvm::Function::ExternalLinkage,
                "rand", &_builtinModule
        );

        auto defaultMin = getConstantNum(-1, -1, getConstantForm(MaximAst::Form::Type::LINEAR, {0, 1}));
        auto defaultMax = getConstantNum(1, 1, getConstantForm(MaximAst::Form::Type::LINEAR, {0, 1}));
        auto func = addFunc("noise", std::make_unique<FunctionDeclaration>(
                true, _numType, std::vector<Parameter> {
                        Parameter(false, _numType, defaultMin),
                        Parameter(false, _numType, defaultMax)
                }
        ), &_builtinModule);

        auto minVal = NumValue::fromRegister(false, func->llFunc()->arg_begin() + 0, this, func);
        auto maxVal = NumValue::fromRegister(false, func->llFunc()->arg_begin() + 1, this, func);

        auto cb = func->codeBuilder();
        auto minNum = cb.CreateLoad(minVal->valuePtr(cb), "min_num");
        auto maxNum = cb.CreateLoad(maxVal->valuePtr(cb), "max_num");
        auto magnitude = cb.CreateFSub(maxNum, minNum, "magnitude");

        auto randValLeft = cb.CreateVectorSplat(2, cb.CreateCall(randIntrinsic, {}, "rand_left_val"), "rand_val_splat");
        auto randVal = cb.CreateInsertElement(
                randValLeft,
                cb.CreateCall(randIntrinsic, {}, "rand_right_val"),
                (uint64_t) 1,
                "rand_val"
        );
        auto randFloatVal = cb.CreateSIToFP(randVal, vecType, "rand_num");

        // todo: probably shouldn't use RAND_MAX here as it's compiler-dependent
        auto randNormalized = cb.CreateFDiv(
                randFloatVal,
                llvm::ConstantVector::get({ getConstantFloat(RAND_MAX), getConstantFloat(RAND_MAX) }),
                "rand_normalized"
        );
        auto randMag = cb.CreateFMul(randNormalized, magnitude, "rand_mag");
        auto randRes = cb.CreateFAdd(randMag, minNum, "rand_result");

        auto newNum = std::make_unique<NumValue>(
                false, randRes, FormValue(MaximAst::Form::Type::LINEAR, {}, this, func),
                this, func
        );
        cb.CreateRet(cb.CreateLoad(newNum->value(), "result_load"));

        func->initBuilder().CreateBr(func->codeBlock());
    }

    // filters
    // num lowBqFilter(num x, num freq, num q)
    // num highBqFilter(num x, num freq, num q)
    // num peakBqFilter(num x, num freq, num q, num gain)
    // (num, num, num, num) svFilter(num x, num freq, num q)

    // generators
    // num sinOsc(num freq, num phase)
    // num sqrOsc(num freq, num phase)
    // num sawOsc(num freq, num phase)
    // num triOsc(num freq, num phase)
    // num rmpOsc(num freq, num phase)

    // time
    // num amplitude(num x)
    // num delay(num x, const num d)
    // num resDelay(num x, num d, const num r)
    // num next(num x)
    // num hold(num x, num gate, num else=0)
    // num accum(num x, num gate, num base=0)
}
