#include "Control.h"

using namespace MaximCodegen;

Control::Control(MaximContext *context, MaximCommon::ControlType type) : _context(context), _type(type) {

}

llvm::Constant* Control::getInitialVal(MaximContext *ctx) {
    return llvm::UndefValue::get(type(ctx));
}

void Control::initializeVal(MaximContext *ctx, llvm::Module *module, llvm::Value *ptr, InstantiableFunction *func, Builder &b) {

}
