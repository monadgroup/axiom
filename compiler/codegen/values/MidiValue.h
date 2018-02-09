#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/DerivedTypes.h>

#include "Value.h"
#include "../Builder.h"

namespace MaximCodegen {

    class Function;

    class Context;

    class MidiValue : public Value {
    public:
        MidiValue(bool isConst, llvm::Value *type, llvm::Value *channel, llvm::Value *note, llvm::Value *param,
                  llvm::Value *time, Context *context, Function *function);

        MidiValue(bool isConst, llvm::Value *value, Context *context);

        llvm::StructType *type() const override;

        llvm::Value *value() const override { return _value; }

        llvm::Value *typePtr(Builder &builder) const;

        llvm::Value *channelPtr(Builder &builder) const;

        llvm::Value *notePtr(Builder &builder) const;

        llvm::Value *paramPtr(Builder &builder) const;

        llvm::Value *timePtr(Builder &builder) const;

        std::unique_ptr<Value> clone() const override;

    private:
        llvm::Value *_value;
        Context *_context;
    };

}
