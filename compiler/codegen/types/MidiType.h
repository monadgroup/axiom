#pragma once

#include <llvm/IR/DerivedTypes.h>

#include "Type.h"

namespace MaximCodegen {

    class MidiType : public Type {
    public:
        explicit MidiType(Context *context);

        llvm::StructType *llType() const override { return _llType; }

        llvm::Type *typeType() const { return _typeType; }

        llvm::Type *channelType() const { return _channelType; }

        llvm::Type *noteType() const { return _noteType; }

        llvm::Type *paramType() const { return _paramType; }

        llvm::Type *timeType() const { return _timeType; }

    private:
        llvm::StructType *_llType;
        llvm::Type *_typeType;
        llvm::Type *_channelType;
        llvm::Type *_noteType;
        llvm::Type *_paramType;
        llvm::Type *_timeType;
    };

}
