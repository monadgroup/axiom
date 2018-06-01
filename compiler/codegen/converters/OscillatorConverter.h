#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class OscillatorConverter : public Converter {
    public:
        OscillatorConverter(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<OscillatorConverter> create(MaximContext *ctx, llvm::Module *module);

    private:
        llvm::Value *fromControl(ComposableModuleClassMethod *method, llvm::Value *val);

    };

}
