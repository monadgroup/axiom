#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class BeatsConverter : public Converter {
    public:
        explicit BeatsConverter(MaximContext *context);

        static std::unique_ptr<BeatsConverter> create(MaximContext *context);

    private:
        llvm::Value *fromSeconds(Builder &b, llvm::Value *val, llvm::Module *module);
    };

}
