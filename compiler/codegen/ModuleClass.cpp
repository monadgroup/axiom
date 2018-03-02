#include "ModuleClass.h"

#include "MaximContext.h"

using namespace MaximCodegen;

size_t ModuleClass::_nextIndex = 0;

ModuleClass::ModuleClass(MaximContext *ctx, llvm::Module *module, const std::string &name)
    : _index(_nextIndex++), _ctx(ctx), _module(module), _name(name) {

}

llvm::Constant* ModuleClass::initializeVal() {
    return llvm::ConstantAggregateZero::get(storageType());
}

llvm::Type* ModuleClass::storageType() {
    return initializeVal()->getType();
}

void ModuleClass::complete() {
    if (_completed) return;
    doComplete();
    _completed = true;
}

std::string ModuleClass::mangleMethodName(const std::string &name) {
    return "maximclass." + _name + "." + std::to_string(_index) + "." + name;
}


void ModuleClass::doComplete() {
    constructor()->builder().CreateRetVoid();
}
