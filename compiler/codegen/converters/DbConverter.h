#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class DbConverter : public Converter {
    public:
        DbConverter(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<DbConverter> create(MaximContext *ctx, llvm::Module *module);

    private:
        llvm::Value *fromAmplitude(ComposableModuleClassMethod *method, llvm::Value *val);

        llvm::Value *fromControl(ComposableModuleClassMethod *method, llvm::Value *val);
    };

}
