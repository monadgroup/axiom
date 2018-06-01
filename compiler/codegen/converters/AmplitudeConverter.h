#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class AmplitudeConverter : public Converter {
    public:
        AmplitudeConverter(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<AmplitudeConverter> create(MaximContext *ctx, llvm::Module *module);

    private:
        llvm::Value *fromDb(ComposableModuleClassMethod *method, llvm::Value *val);
    };

}
