#pragma once

#include "../Control.h"

namespace MaximCodegen {

    class Type;

    class ArrayType;

    class ExtractControl : public Control {
    public:
        explicit ExtractControl(MaximContext *context, MaximCommon::ControlType type, Type *baseType);

        static std::unique_ptr<ExtractControl> create(MaximContext *context, MaximCommon::ControlType type, Type *baseType);

        llvm::Type *type(MaximContext *ctx) const override;

        bool validateProperty(std::string name) override;

        void setProperty(Builder &b, std::string name, std::unique_ptr<Value> val, llvm::Value *ptr) override;

        std::unique_ptr<Value> getProperty(Builder &b, std::string name, llvm::Value *ptr) override;

    private:
        ArrayType *_type;
    };

}
