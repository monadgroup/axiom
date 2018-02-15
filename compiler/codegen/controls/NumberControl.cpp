#include "NumberControl.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

NumberControl::NumberControl(MaximContext *context) : Control(context, MaximCommon::ControlType::NUMBER) {
}

std::unique_ptr<NumberControl> NumberControl::create(MaximContext *context) {
    return std::make_unique<NumberControl>(context);
}

llvm::Constant* NumberControl::getInitialVal(MaximContext *ctx) {
    return llvm::ConstantStruct::get(type(ctx), {
        llvm::ConstantVector::getSplat(2, ctx->constFloat(0)),
        llvm::ConstantInt::get(ctx->numType()->formType(), (uint64_t) MaximCommon::FormType::LINEAR, false),
        llvm::ConstantInt::get(ctx->numType()->activeType(), (uint64_t) false, false)
    });
}

llvm::StructType* NumberControl::type(MaximContext *ctx) const {
    return ctx->numType()->get();
}

bool NumberControl::validateProperty(std::string name) {
    return name == "value";
}

void NumberControl::setProperty(Builder &b, std::string name, std::unique_ptr<Value> val, llvm::Value *ptr) {
    auto numVal = context()->assertNum(std::move(val));
    b.CreateStore(numVal->get(), ptr);
}

std::unique_ptr<Value> NumberControl::getProperty(Builder &b, std::string name, llvm::Value *ptr) {
    auto undefPos = SourcePos(-1, -1);
    auto numVal = Num::create(context(), b.CreateLoad(ptr, "control"), undefPos, undefPos);
    return numVal->withForm(b, MaximCommon::FormType::CONTROL, undefPos, undefPos)->withActive(b, true, undefPos, undefPos);
}
