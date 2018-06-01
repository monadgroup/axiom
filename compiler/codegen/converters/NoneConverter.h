#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class NoneConverter : public Converter {
    public:
        NoneConverter(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<NoneConverter> create(MaximContext *ctx, llvm::Module *module);
    };

}
