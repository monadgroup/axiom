#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class LinearConverter : public Converter {
    public:
        explicit LinearConverter(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<LinearConverter> create(MaximContext *ctx, llvm::Module *module);
    };

}
