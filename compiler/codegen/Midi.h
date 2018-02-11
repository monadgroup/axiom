#pragma once

#include "../common/MidiEventType.h"
#include "Value.h"
#include "MidiType.h"
#include "Builder.h"

namespace llvm {
    class Value;
}

namespace MaximCodegen {

    class Midi : public Value {
    public:
        Midi(MaximContext *context, MaximCommon::MidiEventType event, uint8_t channel, uint8_t note, uint8_t param,
             SourcePos startPos, SourcePos endPos);

        Midi(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos);

        static std::unique_ptr<Midi>
        create(MaximContext *context, MaximCommon::MidiEventType event, uint8_t channel, uint8_t note, uint8_t param,
               SourcePos startPos, SourcePos endPos);

        static std::unique_ptr<Midi>
        create(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos);

        llvm::Value *get() const override { return _get; }

        llvm::Value *event(Builder &builder) const;

        llvm::Value *channel(Builder &builder) const;

        llvm::Value *note(Builder &builder) const;

        llvm::Value *param(Builder &builder) const;

        MidiType *type() const override;

    private:
        llvm::Value *_get;
        MaximContext *_context;
    };

}
