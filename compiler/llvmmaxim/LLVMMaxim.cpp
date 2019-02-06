#include <llvm-c/Core.h>
#include <llvm-c/OrcBindings.h>
#include <llvm-c/TargetMachine.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>

#include "OrcJit.h"

DEFINE_SIMPLE_CONVERSION_FUNCTIONS(std::shared_ptr<llvm::Module>, LLVMSharedModuleRef)
DEFINE_SIMPLE_CONVERSION_FUNCTIONS(llvm::TargetMachine, LLVMTargetMachineRef)

extern "C" {
LLVMTargetMachineRef LLVMAxiomSelectTarget() {
    return wrap(llvm::EngineBuilder().selectTarget());
}

// Builder utilities
void LLVMAxiomSetFastMathFlags(LLVMBuilderRef builder, bool allowReassoc, bool noNans, bool noInfs, bool noSignedZeros,
                               bool allowReciprocal, bool allowContract, bool approxFunc) {
    llvm::FastMathFlags flags;
    if (allowReassoc) flags.setAllowReassoc();
    if (noNans) flags.setNoNaNs();
    if (noInfs) flags.setNoInfs();
    if (noSignedZeros) flags.setNoSignedZeros();
    if (allowReciprocal) flags.setAllowReciprocal();
    flags.setAllowContract(allowContract);
    if (approxFunc) flags.setApproxFunc();

    llvm::unwrap(builder)->setFastMathFlags(flags);
}

void LLVMAxiomSetAllFastMathFlags(LLVMBuilderRef builder) {
    llvm::FastMathFlags flags;
    flags.setFast();
    llvm::unwrap(builder)->setFastMathFlags(flags);
}

// JIT functions
OrcJit *LLVMAxiomOrcCreateInstance(LLVMTargetMachineRef targetMachine) {
    auto jit = new OrcJit(*unwrap(targetMachine));

    // libc memory functions
    jit->addBuiltin("memcpy", (uint64_t) & ::memcpy);
    jit->addBuiltin("memset", (uint64_t) & ::memset);
    jit->addBuiltin("realloc", (uint64_t) & ::realloc);
    jit->addBuiltin("free", (uint64_t) & ::free);

    return jit;
}

void LLVMAxiomOrcAddBuiltin(OrcJit *jit, const char *name, LLVMOrcTargetAddress address) {
    jit->addBuiltin(name, address);
}

LLVMOrcModuleHandle LLVMAxiomOrcAddModule(OrcJit *jit, LLVMSharedModuleRef module) {
    return jit->addModule(std::move(*unwrap(module)));
}

void LLVMAxiomOrcRemoveModule(OrcJit *jit, LLVMOrcModuleHandle handle) {
    jit->removeModule(handle);
}

LLVMOrcTargetAddress LLVMAxiomOrcGetSymbolAddress(OrcJit *jit, const char *name) {
    return jit->getSymbolAddress(name);
}

void LLVMAxiomOrcDisposeInstance(OrcJit *jit) {
    delete jit;
}
}
