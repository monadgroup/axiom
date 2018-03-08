#pragma once

#include <llvm/IR/DerivedTypes.h>

#include "Type.h"

namespace llvm {
    class StructLayout;
}

namespace MaximCodegen {

    class MaximContext;

    class MidiType : public Type {
    public:
        static constexpr size_t maxEvents = 32;

        explicit MidiType(MaximContext *context);

        llvm::StructType *get() const override { return _type; }

        llvm::Type *countType() const { return _countType; }

        llvm::ArrayType *arrayType() const { return _arrayType; }

        llvm::StructType *eventType() const { return _eventType; }

        llvm::Type *typeType() const { return _typeType; }

        llvm::Type *channelType() const { return _channelType; }

        llvm::Type *noteType() const { return _noteType; }

        llvm::Type *paramType() const { return _paramType; }

        const llvm::StructLayout *layout() const { return _layout; }

        const llvm::StructLayout *eventLayout() const { return _eventLayout; }

        std::string name() const override { return "midi"; }

        std::unique_ptr<Value> createInstance(llvm::Value *val, SourcePos startPos, SourcePos endPos) override;

    private:
        MaximContext *_context;

        llvm::StructType *_type;

        llvm::Type *_countType;

        llvm::ArrayType *_arrayType;

        llvm::StructType *_eventType;

        llvm::Type *_typeType;

        llvm::Type *_channelType;

        llvm::Type *_noteType;

        llvm::Type *_paramType;

        const llvm::StructLayout *_layout;

        const llvm::StructLayout *_eventLayout;
    };

}
