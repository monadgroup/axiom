#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class SecondsConverter : public Converter {
    public:
        explicit SecondsConverter(MaximContext *context);

        static std::unique_ptr<SecondsConverter> create(MaximContext *context);

    private:
        llvm::Value *fromBeats(Builder &b, llvm::Value *val, llvm::Module *module);

        llvm::Value *fromControl(Builder &b, llvm::Value *val, llvm::Module *module);

        llvm::Value *fromFrequency(Builder &b, llvm::Value *val, llvm::Module *module);
    };

}
