#include <ctime>
#include <llvm-c/Core.h>
#include <llvm-c/OrcBindings.h>
#include <llvm-c/TargetMachine.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>

#include "OrcJit.h"

DEFINE_SIMPLE_CONVERSION_FUNCTIONS(std::shared_ptr<llvm::Module>, LLVMSharedModuleRef)
DEFINE_SIMPLE_CONVERSION_FUNCTIONS(llvm::TargetMachine, LLVMTargetMachineRef)

#ifdef APPLE
#define SINCOSF ::__sincosf
#else
#define SINCOSF ::sincosf
#endif

extern "C" {
long __moddi3(long a, long b);
unsigned long __umoddi3(unsigned long a, unsigned long b);

LLVMTargetMachineRef LLVMAxiomSelectTarget() {
    auto target = llvm::EngineBuilder().selectTarget();
    target->Options.AllowFPOpFusion = llvm::FPOpFusion::Fast;
    target->Options.FPDenormalMode = llvm::FPDenormal::PositiveZero;
    return wrap(target);
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

uint64_t profileTimestamp() {
    return clock();
}

// JIT functions
OrcJit *LLVMAxiomOrcCreateInstance(LLVMTargetMachineRef targetMachine) {
    auto jit = new OrcJit(*unwrap(targetMachine));

    jit->addBuiltin("memcpy", (uint64_t) & ::memcpy);
    jit->addBuiltin("powf", (uint64_t) & ::powf);
    jit->addBuiltin("logf", (uint64_t) & ::logf);
    jit->addBuiltin("log10f", (uint64_t) & ::log10f);
    jit->addBuiltin("log2f", (uint64_t) & ::log2f);
    jit->addBuiltin("cosf", (uint64_t) & ::cosf);
    jit->addBuiltin("sinf", (uint64_t) & ::sinf);
    jit->addBuiltin("sqrtf", (uint64_t) & ::sqrtf);
    jit->addBuiltin("ceilf", (uint64_t) & ::ceilf);
    jit->addBuiltin("floorf", (uint64_t) & ::floorf);
    jit->addBuiltin("fabsf", (uint64_t) & ::fabsf);
    jit->addBuiltin("fminf", (uint64_t) & ::fminf);
    jit->addBuiltin("fmaxf", (uint64_t) & ::fmaxf);
    jit->addBuiltin("exp2f", (uint64_t) & ::exp2f);
    jit->addBuiltin("tanf", (uint64_t) & ::tanf);
    jit->addBuiltin("acosf", (uint64_t) & ::acosf);
    jit->addBuiltin("asinf", (uint64_t) & ::asinf);
    jit->addBuiltin("atanf", (uint64_t) & ::atanf);
    jit->addBuiltin("atan2f", (uint64_t) & ::atan2f);
    jit->addBuiltin("hypot", (uint64_t) static_cast<double (*)(double, double)>(&::hypot));
    jit->addBuiltin("sincosf", (uint64_t) &SINCOSF);
    jit->addBuiltin("expf", (uint64_t) & ::expf);
    jit->addBuiltin("fmodf", (uint64_t) & ::fmodf);
    jit->addBuiltin("rand", (uint64_t) & ::rand);
    jit->addBuiltin("realloc", (uint64_t) & ::realloc);
    jit->addBuiltin("free", (uint64_t) & ::free);
    jit->addBuiltin("memset", (uint64_t) & ::memset);
    jit->addBuiltin("__moddi3", (uint64_t) & ::__moddi3);
    jit->addBuiltin("__umoddi3", (uint64_t) & ::__umoddi3);

    jit->addBuiltin("profile_timestamp", (uint64_t) & ::profileTimestamp);

#ifdef APPLE
    jit->addBuiltin("__sincosf_stret", (uint64_t) & ::__sincosf_stret);
    jit->addBuiltin("__bzero", (uint64_t) & ::bzero);
#endif

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
