#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class NoteConverter : public Converter {
    public:
        NoteConverter(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<NoteConverter> create(MaximContext *ctx, llvm::Module *module);

    private:
        llvm::Value *fromControl(ComposableModuleClassMethod *method, llvm::Value *val);

        llvm::Value *fromFrequency(ComposableModuleClassMethod *method, llvm::Value *val);
    };

}
