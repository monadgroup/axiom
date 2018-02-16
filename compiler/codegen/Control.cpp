#include "Control.h"

using namespace MaximCodegen;

Control::Control(MaximContext *context, MaximCommon::ControlType type) : _context(context), _type(type) {

}

void Control::initializeVal(MaximContext *ctx, llvm::Module *module, llvm::Value *ptr, Builder &b) {
    b.CreateStore(storagePointer, ptr);
}
