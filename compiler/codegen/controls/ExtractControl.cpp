#include "ExtractControl.h"

#include "../MaximContext.h"
#include "../Array.h"

using namespace MaximCodegen;

ExtractControl::ExtractControl(MaximContext *context, MaximCommon::ControlType type, Type *baseType)
    : Control(context, type), _type(context->getArrayType(baseType)) {

}

std::unique_ptr<ExtractControl> ExtractControl::create(MaximContext *context, MaximCommon::ControlType type, Type *baseType) {
    return std::make_unique<ExtractControl>(context, type, baseType);
}

llvm::Type* ExtractControl::type(MaximContext *ctx) const {
    return llvm::PointerType::get(_type->get(), 0);
}

bool ExtractControl::validateProperty(std::string name) {
    return name == "value";
}

void ExtractControl::setProperty(Builder &b, std::string name, std::unique_ptr<Value> val, llvm::Value *ptr) {
    auto arrayVal = context()->assertArray(std::move(val), _type);
    b.CreateStore(arrayVal->get(), b.CreateLoad(ptr, "ptr"));
}

std::unique_ptr<Value> ExtractControl::getProperty(Builder &b, std::string name, llvm::Value *ptr) {
    auto undefPos = SourcePos(-1, -1);
    return Array::create(context(), _type, b.CreateLoad(b.CreateLoad(ptr, "ptr"), "control"), undefPos, undefPos);
}
