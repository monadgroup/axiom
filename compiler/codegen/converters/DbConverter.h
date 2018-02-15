#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class DbConverter : public Converter {
    public:
        explicit DbConverter(MaximContext *context);

        static std::unique_ptr<DbConverter> create(MaximContext *context);

    private:
        llvm::Value *fromControl(Builder &b, llvm::Value *val, llvm::Module *module);
    };

}
