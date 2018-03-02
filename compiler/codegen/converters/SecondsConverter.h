#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class SecondsConverter : public Converter {
    public:
        explicit SecondsConverter(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<SecondsConverter> create(MaximContext *ctx, llvm::Module *module);

    private:
        llvm::Value *fromBeats(ComposableModuleClassMethod *method, llvm::Value *val);

        llvm::Value *fromControl(ComposableModuleClassMethod *method, llvm::Value *val);

        llvm::Value *fromFrequency(ComposableModuleClassMethod *method, llvm::Value *val);
    };

}
