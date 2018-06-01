#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class QConverter : public Converter {
    public:
        QConverter(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<QConverter> create(MaximContext *ctx, llvm::Module *module);

    private:
        llvm::Value *fromControl(ComposableModuleClassMethod *method, llvm::Value *value);

    };

}
