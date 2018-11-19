#include <ctime>
#include <llvm-c/Core.h>
#include <llvm-c/OrcBindings.h>
#include <llvm-c/TargetMachine.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Target/TargetIntrinsicInfo.h>

#include "OrcJit.h"

DEFINE_SIMPLE_CONVERSION_FUNCTIONS(std::shared_ptr<llvm::Module>, LLVMSharedModuleRef)
DEFINE_SIMPLE_CONVERSION_FUNCTIONS(llvm::TargetMachine, LLVMTargetMachineRef)

#ifdef APPLE
#define SINCOS ::__sincos
#else
#define SINCOS ::sincos
#endif

extern "C" {
long __moddi3(long a, long b);
unsigned long __umoddi3(unsigned long a, unsigned long b);

LLVMTargetMachineRef LLVMAxiomSelectTarget() {
    auto target = llvm::EngineBuilder().selectTarget();
    target->Options.AllowFPOpFusion = llvm::FPOpFusion::Fast;
    // target->Options.FPDenormalMode = llvm::FPDenormal::PositiveZero;
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

    // utilities
    jit->addBuiltin("profile_timestamp", (uint64_t) & ::profileTimestamp);

    // libc memory functions
    jit->addBuiltin("memcpy", (uint64_t) & ::memcpy);
    jit->addBuiltin("memset", (uint64_t) & ::memset);
    jit->addBuiltin("realloc", (uint64_t) & ::realloc);
    jit->addBuiltin("free", (uint64_t) & ::free);

    // math functions
    // todo: replace all of these with our own SSE-based intrinsics
    jit->addBuiltin("rand", (uint64_t) & ::rand);
    jit->addBuiltin("cos", (uint64_t) & ::cos);
    jit->addBuiltin("sin", (uint64_t) & ::sin);
    jit->addBuiltin("sincos", (uint64_t) &SINCOS);
    jit->addBuiltin("tan", (uint64_t) & ::tan);
    jit->addBuiltin("atan", (uint64_t) & ::atan);
    jit->addBuiltin("atan2", (uint64_t) & ::atan2);
    jit->addBuiltin("acos", (uint64_t) & ::acos);
    jit->addBuiltin("asin", (uint64_t) & ::asin);
    jit->addBuiltin("hypot", (uint64_t) static_cast<double (*)(double, double)>(&::hypot));
    jit->addBuiltin("pow", (uint64_t) & ::pow);
    jit->addBuiltin("exp", (uint64_t) & ::exp);
    jit->addBuiltin("exp2", (uint64_t) & ::exp2);
    jit->addBuiltin("log", (uint64_t) & ::log);
    jit->addBuiltin("log2", (uint64_t) & ::log2);
    jit->addBuiltin("log10", (uint64_t) & ::log10);
    jit->addBuiltin("__moddi3", (uint64_t) & ::__moddi3);
    jit->addBuiltin("__umoddi3", (uint64_t) & ::__umoddi3);

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
