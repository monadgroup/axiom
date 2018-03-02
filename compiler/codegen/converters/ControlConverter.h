#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class ControlConverter : public Converter {
    public:
        explicit ControlConverter(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<ControlConverter> create(MaximContext *ctx, llvm::Module *module);

    private:
        llvm::Value *fromOscillator(ComposableModuleClassMethod *method, llvm::Value *val);

        llvm::Value *fromFrequency(ComposableModuleClassMethod *method, llvm::Value *val);

        llvm::Value *fromNote(ComposableModuleClassMethod *method, llvm::Value *val);

        llvm::Value *fromDb(ComposableModuleClassMethod *method, llvm::Value *val);

        llvm::Value *fromQ(ComposableModuleClassMethod *method, llvm::Value *val);

        llvm::Value *fromSeconds(ComposableModuleClassMethod *method, llvm::Value *val);

        llvm::Value *fromBeats(ComposableModuleClassMethod *method, llvm::Value *val);
    };

}
