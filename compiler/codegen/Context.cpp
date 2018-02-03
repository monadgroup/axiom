#include "Context.h"

#include "CodegenError.h"

#include "values/NumValue.h"
#include "values/MidiValue.h"
#include "values/TupleValue.h"

using namespace MaximCodegen;

Context::Context() {
    llvm::StructType::create(_llvm, std::array<llvm::Type *, 2> {
            llvm::Type::getInt8Ty(_llvm), // type
            llvm::ArrayType::get(llvm::Type::getFloatTy(_llvm), formParamCount) // form params
    }, "form");

    llvm::StructType::create(_llvm, std::array<llvm::Type *, 2> {
        llvm::VectorType::get(
            llvm::Type::getFloatTy(_llvm),
            2
        ),

        getStructType(Type::FORM)
    }, "num");

    llvm::StructType::create(_llvm, std::array<llvm::Type *, 5> {
        llvm::Type::getIntNTy(_llvm, 4), // event type
        llvm::Type::getIntNTy(_llvm, 4), // channel
        llvm::Type::getInt8Ty(_llvm),    // note
        llvm::Type::getInt8Ty(_llvm),    // param
        llvm::Type::getInt32Ty(_llvm)    // time
    }, "midi");
}

llvm::Constant* Context::getConstantInt(unsigned int numBits, uint64_t val, bool isSigned) {
    return llvm::ConstantInt::get(_llvm, llvm::APInt(numBits, val, isSigned));
}

llvm::Constant* Context::getConstantFloat(float num) {
    return llvm::ConstantFP::get(_llvm, llvm::APFloat(num));
}

llvm::Value* Context::getStructParamPtr(llvm::Value *ptr, llvm::Type *type, unsigned int param,
                                        llvm::IRBuilder<> &builder) {
    return builder.Insert(llvm::GetElementPtrInst::Create(
            type, ptr,
            std::array<llvm::Value *, 2> {
                    getConstantInt(32, 0, false),
                    getConstantInt(32, param, false)
            }
    ));
}

llvm::Value* Context::checkType(llvm::Value *val, llvm::Type *type, SourcePos start, SourcePos end) {
    if (val->getType() != type) {
        throw CodegenError(
                "Oyyyyy m80, I need a " + typeToString(type) + " here, not this bad boi " + typeToString(val->getType()),
                start, end
        );
    }
    return val;
}

llvm::Value* Context::checkType(llvm::Value *val, Type type, SourcePos start, SourcePos end) {
    return checkType(val, getType(type), start, end);
}

llvm::Type *Context::getType(Type type) {
    switch (type) {
        case Type::FLOAT: return llvm::Type::getFloatTy(_llvm);
        case Type::INT4: return llvm::Type::getIntNTy(_llvm, 4);
        case Type::INT8: return llvm::Type::getInt8Ty(_llvm);
        case Type::INT32: return llvm::Type::getInt32Ty(_llvm);
        default: return getStructType(type);
    }
}

llvm::StructType* Context::getStructType(Type type) {
    switch (type) {
        case Type::FORM: return llvm::StructType::create(_llvm, "form");
        case Type::NUM: return llvm::StructType::create(_llvm, "num");
        case Type::MIDI: return llvm::StructType::create(_llvm, "midi");
        default: assert(false);
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
        case Type::FLOAT: return "float";
        case Type::INT4: return "i4";
        case Type::INT8: return "i8";
        case Type::INT32: return "i32";
        case Type::FORM: return "form";
        case Type::NUM: return "num";
        case Type::MIDI: return "midi";
        case Type::TUPLE: return "tuple";
    }
}

std::unique_ptr<Value> Context::llToValue(bool isConst, llvm::Value *value) {
    auto type = getType(value->getType());
    switch (type) {
        case Type::NUM: return std::make_unique<NumValue>(isConst, value, this);
        case Type::MIDI: return std::make_unique<MidiValue>(isConst, value, this);
        case Type::TUPLE: return std::make_unique<TupleValue>(isConst, value, this);
        default: assert(false);
    }

    throw;
}
