#pragma once

#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

namespace llvm {
    class Target;
}

namespace MaximCodegen {
    class MaximContext;
}

namespace MaximRuntime {

    class Runtime;

    class Exporter {
    public:
        Exporter(MaximCodegen::MaximContext *context, Runtime *runtime);

        void addRuntime(Runtime *runtime, const std::string &exportName);

        void exportObject(llvm::raw_fd_ostream &dest, unsigned optLevel, unsigned sizeLevel);

        void exportLto(llvm::raw_fd_ostream &dest, unsigned optLevel, unsigned sizeLevel);

    private:
        llvm::Module module;
        llvm::TargetMachine *target;
    };

}
