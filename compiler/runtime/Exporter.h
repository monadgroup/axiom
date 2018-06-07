#pragma once

#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

namespace llvm {
    class Target;
}

namespace MaximCodegen {
    class MaximContext;

    class ModuleClassMethod;
}

namespace MaximRuntime {

    class Runtime;

    class Exporter {
    public:
        Exporter(MaximCodegen::MaximContext *context, const llvm::Module *commonModule);

        void addRuntime(Runtime *runtime, const std::string &exportName);

        void exportObject(llvm::raw_fd_ostream &dest, unsigned optLevel, unsigned sizeLevel);

        void exportLto(llvm::raw_fd_ostream &dest, unsigned optLevel, unsigned sizeLevel);

    private:
        llvm::Module module;
        llvm::TargetMachine *target;

        llvm::StructType *_exportDefinitionTy;
        llvm::StructType *_exportInstrumentTy;

        void finishModule(unsigned optLevel, unsigned sizeLevel);

        void buildInterfaceFunctions(MaximCodegen::MaximContext *ctx);

        void buildCreateInstrumentFunc(MaximCodegen::MaximContext *ctx);

        void buildGetInputFunc(MaximCodegen::MaximContext *ctx);

        void buildGetOutputFunc(MaximCodegen::MaximContext *ctx);

        void buildGenerateFunc(MaximCodegen::MaximContext *ctx);

        void buildDestroyInstrumentFunc(MaximCodegen::MaximContext *ctx);

        void buildMidiPushFunc(MaximCodegen::MaximContext *ctx);

        void buildMidiClearFunc(MaximCodegen::MaximContext *ctx);

        void buildNumWriteFunc(MaximCodegen::MaximContext *ctx);

        void buildNumReadFunc(MaximCodegen::MaximContext *ctx);

        llvm::Function *buildInstrumentFunc(MaximCodegen::MaximContext *ctx, const std::string &name, MaximCodegen::ModuleClassMethod *method);
    };

}
