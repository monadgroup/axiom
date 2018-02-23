#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class FrequencyConverter : public Converter {
    public:
        explicit FrequencyConverter(MaximContext *context);

        static std::unique_ptr<FrequencyConverter> create(MaximContext *context);

    private:
        llvm::Value *fromControl(Builder &b, llvm::Value *val, llvm::Module *module);
        llvm::Value *fromSeconds(Builder &b, llvm::Value *val, llvm::Module *module);
        llvm::Value *fromNote(Builder &b, llvm::Value *val, llvm::Module *module);
    };

}
