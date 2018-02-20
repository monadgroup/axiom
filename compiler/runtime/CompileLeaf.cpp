#include "CompileLeaf.h"

#include "Runtime.h"

using namespace MaximRuntime;

CompileLeaf::CompileLeaf(Runtime *runtime) : _runtime(runtime) {

}

void CompileLeaf::updateGetter(llvm::Module *module) {
    if (!parentUnit()) return;

    if (getValueFunction) getValueFunction->removeFromParent();

    auto parentInt = parentUnit()->inst();

    auto selfContextType = inst()->type(_runtime->context());
    auto parentContextType = parentInt->type(_runtime->context());
    getValueFunction = llvm::Function::Create(
        llvm::FunctionType::get(llvm::PointerType::get(selfContextType, 0), {llvm::PointerType::get(parentContextType, 0)}, false),
        llvm::Function::LinkageTypes::ExternalLinkage, "getvalue." + std::to_string(inst()->id()), module
    );
    auto getValueBlock = llvm::BasicBlock::Create(_runtime->context()->llvm(), "entry", getValueFunction);
    MaximCodegen::Builder b(getValueBlock);

    auto selfIndex = std::distance(
        parentInt->instantiables().begin(),
        std::find(parentInt->instantiables().begin(), parentInt->instantiables().end(), inst())
    );
    assert(selfIndex != (ssize_t) parentInt->instantiables().size());

    auto gep = b.CreateStructGEP(parentContextType, getValueFunction->arg_begin(), (unsigned int) selfIndex);
    b.CreateRet(gep);

    getValueFunction->print(llvm::errs(), nullptr);
}

void CompileLeaf::deploy() {
    if (getValueFunction) {
        _getValuePtr = (void *(*)(void *)) _runtime->jit.getSymbolAddress(getValueFunction);
        assert(_getValuePtr);
    }
}

void* CompileLeaf::getValuePtr(void *parentCtx) {
    if (_getValuePtr == nullptr || parentCtx == nullptr) return nullptr;
    return _getValuePtr(parentCtx);
}

void *CompileLeaf::updateCurrentPtr(void *parentCtx) {
    _currentPtr = getValuePtr(parentCtx);
    assert(_currentPtr);
    return currentPtr();
}

void* CompileLeaf::currentPtr() const {
    return _currentPtr;
}
