#include "NumberControl.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

NumberControl::NumberControl(MaximContext *context) : Control(context, MaximCommon::ControlType::NUMBER) {
}

std::unique_ptr<NumberControl> NumberControl::create(MaximContext *context) {
    return std::make_unique<NumberControl>(context);
}

llvm::Type *NumberControl::type(MaximContext *ctx) const {
    return llvm::PointerType::get(ctx->numType()->get(), 0);
}

bool NumberControl::validateProperty(std::string name) {
    return name == "value";
}

void NumberControl::setProperty(Builder &b, std::string name, std::unique_ptr<Value> val, llvm::Value *ptr) {
    auto numVal = context()->assertNum(std::move(val));
    b.CreateStore(numVal->get(), b.CreateLoad(ptr, "ptr"));
}

std::unique_ptr<Value> NumberControl::getProperty(Builder &b, std::string name, llvm::Value *ptr) {
    auto undefPos = SourcePos(-1, -1);
    auto numVal = Num::create(context(), b.CreateLoad(b.CreateLoad(ptr, "ptr"), "control"), undefPos, undefPos);
    return numVal->withForm(b, MaximCommon::FormType::CONTROL, undefPos, undefPos)->withActive(b, true, undefPos,
                                                                                               undefPos);
}
