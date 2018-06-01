#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class SamplesConverter : public Converter {
    public:
        SamplesConverter(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<SamplesConverter> create(MaximContext *ctx, llvm::Module *module);

    private:
        llvm::Value *fromBeats(ComposableModuleClassMethod *method, llvm::Value *val);

        llvm::Value *fromControl(ComposableModuleClassMethod *method, llvm::Value *val);

        llvm::Value *fromFrequency(ComposableModuleClassMethod *method, llvm::Value *val);

        llvm::Value *fromSeconds(ComposableModuleClassMethod *method, llvm::Value *val);
    };

}
