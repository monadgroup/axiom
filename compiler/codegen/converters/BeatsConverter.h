#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class BeatsConverter : public Converter {
    public:
        BeatsConverter(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<BeatsConverter> create(MaximContext *ctx, llvm::Module *module);

    private:
        llvm::Value *fromControl(ComposableModuleClassMethod *method, llvm::Value *val);

        llvm::Value *fromFrequency(ComposableModuleClassMethod *method, llvm::Value *val);

        llvm::Value *fromSamples(ComposableModuleClassMethod *method, llvm::Value *val);

        llvm::Value *fromSeconds(ComposableModuleClassMethod *method, llvm::Value *val);
    };

}
