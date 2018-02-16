#pragma once

#include "../common/ControlType.h"
#include "../common/ControlDirection.h"
#include "Instantiable.h"

namespace MaximCodegen {

    class MaximContext;

    class Value;

    class Control : public Instantiable {
    public:
        MaximCommon::ControlDirection direction = MaximCommon::ControlDirection::NONE;

        llvm::Value *storagePointer;

        Control(MaximContext *context, MaximCommon::ControlType type);

        MaximCommon::ControlType type() const { return _type; }

        virtual bool validateProperty(std::string name) = 0;

        virtual void setProperty(Builder &b, std::string name, std::unique_ptr<Value> val, llvm::Value *ptr) = 0;

        virtual std::unique_ptr<Value> getProperty(Builder &b, std::string name, llvm::Value *ptr) = 0;

        void initializeVal(MaximContext *ctx, llvm::Module *module, llvm::Value *ptr, Builder &b) override;

    protected:
        MaximContext *context() const { return _context; }

    private:
        MaximContext *_context;
        MaximCommon::ControlType _type;
    };

}
