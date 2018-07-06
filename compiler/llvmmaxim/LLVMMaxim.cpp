#include "OrcCBindingsStack.h"
#include <llvm-c/TargetMachine.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>

extern "C" {
LLVMTargetMachineRef LLVMAxiomSelectTarget() {
    return reinterpret_cast<LLVMTargetMachineRef>(llvm::EngineBuilder().selectTarget());
}

LLVMOrcErrorCode LLVMAxiomOrcGetSymbolAddress(LLVMOrcJITStackRef JITStack, LLVMOrcTargetAddress *RetAddr,
                                              const char *SymbolName) {
    auto &j = *reinterpret_cast<llvm::OrcCBindingsStack *>(JITStack);
    return j.findSymbolAddress(*RetAddr, SymbolName, false);
}
}
