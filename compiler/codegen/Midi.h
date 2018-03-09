#pragma once

#include "../common/MidiEventType.h"
#include "Value.h"
#include "MidiType.h"
#include "Builder.h"

namespace llvm {
    class Value;
}

namespace MaximCodegen {

    class MidiEvent {
    public:
        explicit MidiEvent(llvm::Value *get);

        llvm::Value *get() const { return _get; }

        llvm::Value *type(Builder &builder) const;

        llvm::Value *channel(Builder &builder) const;

        llvm::Value *note(Builder &builder) const;

        llvm::Value *param(Builder &builder) const;

        MidiEvent withType(Builder &builder, llvm::Value *type) const;

        MidiEvent withChannel(Builder &builder, llvm::Value *channel) const;

        MidiEvent withNote(Builder &builder, llvm::Value *note) const;

        MidiEvent withParam(Builder &builder, llvm::Value *param) const;

    private:
        llvm::Value *_get;
    };

    class Midi : public Value {
    public:
        Midi(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos);

        static std::unique_ptr<Midi> create(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos);

        llvm::Value *get() const override { return _get; }

        llvm::Value *count(Builder &builder) const;

        llvm::Value *events(Builder &builder) const;

        MidiEvent eventAt(Builder &builder, unsigned index) const;

        MidiEvent eventAt(Builder &builder, llvm::Value *index) const;

        std::unique_ptr<Midi> withCount(Builder &builder, llvm::Value *count, SourcePos startPos, SourcePos endPos) const;

        std::unique_ptr<Midi> withEvents(Builder &builder, llvm::Value *events, SourcePos startPos, SourcePos endPos) const;

        std::unique_ptr<Midi> withEvent(Builder &builder, unsigned index, const MidiEvent &event, SourcePos startPos, SourcePos endPos) const;

        std::unique_ptr<Midi> withEvent(Builder &builder, llvm::Value *index, const MidiEvent &event, SourcePos startPos, SourcePos endPos) const;

        std::unique_ptr<Value> withSource(SourcePos startPos, SourcePos endPos) const override;

        MidiType *type() const override;

    private:
        llvm::Value *_get;
        MaximContext *_context;
    };

}
