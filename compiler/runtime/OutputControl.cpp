#include "OutputControl.h"

#include "../codegen/MaximContext.h"
#include "OutputNode.h"

using namespace MaximRuntime;

OutputControl::OutputControl(OutputNode *node) : Control(node, "output", MaximCommon::ControlType::NUMBER) {

}

llvm::Constant* OutputControl::getInitialVal(MaximCodegen::MaximContext *ctx) {
    return llvm::UndefValue::get(type(ctx));
}

llvm::Type* OutputControl::type(MaximCodegen::MaximContext *ctx) const {
    return llvm::PointerType::get(ctx->numType()->get(), 0);
}
