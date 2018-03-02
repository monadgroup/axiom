#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class DbConverter : public Converter {
    public:
        explicit DbConverter(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<DbConverter> create(MaximContext *ctx, llvm::Module *module);

    private:
        llvm::Value *fromControl(ComposableModuleClassMethod *method, llvm::Value *val);
    };

}
