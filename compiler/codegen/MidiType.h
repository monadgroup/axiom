#pragma once

#include <llvm/IR/DerivedTypes.h>

#include "Type.h"

namespace MaximCodegen {

    class MaximContext;

    class MidiType : public Type {
    public:
        explicit MidiType(MaximContext *context);

        llvm::StructType *get() const override { return _type; }

        llvm::Type *eventType() const { return _eventType; }

        llvm::Type *channelType() const { return _channelType; }

        llvm::Type *noteType() const { return _noteType; }

        llvm::Type *paramType() const { return _paramType; }

        std::string name() const override { return "midi"; }

        std::unique_ptr<Value> createInstance(llvm::Value *val, SourcePos startPos, SourcePos endPos) override;

    private:
        MaximContext *_context;

        llvm::StructType *_type;

        llvm::Type *_eventType;

        llvm::Type *_channelType;

        llvm::Type *_noteType;

        llvm::Type *_paramType;
    };

}
