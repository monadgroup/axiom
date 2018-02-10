#pragma once

#include "Value.h"

namespace llvm {
    class Value;
}

namespace MaximCodegen {

    class Midi : public Value {
    public:
        Midi(llvm::Value *event, llvm::Value *channel, llvm::Value *note, llvm::Value *param, SourcePos startPos,
             SourcePos endPos);

        Midi(MaximCommon::MidiEventType event, uint8_t channel, uint8_t note, uint8_t param, SourcePos startPos,
             SourcePos endPos);

        llvm::Value *get() const override;

        llvm::Value *event() const;

        llvm::Value *channel() const;

        llvm::Value *note() const;

        llvm::Value *param() const;
    };

}
