#include "Exporter.h"

#include <llvm/Support/TargetRegistry.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Bitcode/BitcodeWriter.h>

#include "../codegen/MaximContext.h"
#include "Runtime.h"

using namespace MaximRuntime;

Exporter::Exporter(MaximCodegen::MaximContext *context, Runtime *runtime)
    : module("export", context->llvm()) {
    target = llvm::EngineBuilder().selectTarget();
    module.setTargetTriple(target->getTargetTriple().str());
    module.setDataLayout(target->createDataLayout());

    runtime->generateExportCommon(&module);
}

void Exporter::addRuntime(MaximRuntime::Runtime *runtime, const std::string &exportName) {
    runtime->exportSurface(exportName, &module);
}

void Exporter::exportObject(llvm::raw_fd_ostream &dest, unsigned optLevel, unsigned sizeLevel) {
    Jit::optimizeModule(&module, target, optLevel, sizeLevel);

    llvm::legacy::PassManager pass;
    assert(!target->addPassesToEmitFile(pass, dest, llvm::TargetMachine::CGFT_ObjectFile));
    pass.run(module);
    dest.flush();
}

void Exporter::exportLto(llvm::raw_fd_ostream &dest, unsigned optLevel, unsigned sizeLevel) {
    Jit::optimizeModule(&module, target, optLevel, sizeLevel);

    llvm::WriteBitcodeToFile(&module, dest);
    dest.flush();
}
