#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class ControlConverter : public Converter {
    public:
        explicit ControlConverter(MaximContext *context);

        static std::unique_ptr<ControlConverter> create(MaximContext *context);

    private:
        llvm::Value *fromOscillator(Builder &b, llvm::Value *val, llvm::Module *module);

        llvm::Value *fromFrequency(Builder &b, llvm::Value *val, llvm::Module *module);

        llvm::Value *fromNote(Builder &b, llvm::Value *val, llvm::Module *module);

        llvm::Value *fromDb(Builder &b, llvm::Value *val, llvm::Module *module);

        llvm::Value *fromQ(Builder &b, llvm::Value *val, llvm::Module *module);

        llvm::Value *fromSeconds(Builder &b, llvm::Value *val, llvm::Module *module);

        llvm::Value *fromBeats(Builder &b, llvm::Value *val, llvm::Module *module);
    };

}
