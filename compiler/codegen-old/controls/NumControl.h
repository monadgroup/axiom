#pragma once

#include "../Control.h"

namespace MaximCodegen {

    class NumControl : public Control {
    public:
        explicit NumControl(MaximContext *context);

        static std::unique_ptr<NumControl> create(MaximContext *context);

        llvm::Type *type(MaximContext *ctx) const override;

        bool validateProperty(std::string name) override;

        void setProperty(Builder &b, std::string name, std::unique_ptr<Value> val, llvm::Value *ptr) override;

        std::unique_ptr<Value> getProperty(Builder &b, std::string name, llvm::Value *ptr) override;
    };

}
