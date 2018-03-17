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
        explicit MidiEvent(llvm::Value *get, llvm::Type *type);

        llvm::Value *get() const { return _get; }

        llvm::Value *typePtr(Builder &builder) const;

        llvm::Value *channelPtr(Builder &builder) const;

        llvm::Value *notePtr(Builder &builder) const;

        llvm::Value *paramPtr(Builder &builder) const;

        llvm::Value *type(Builder &builder) const;

        llvm::Value *channel(Builder &builder) const;

        llvm::Value *note(Builder &builder) const;

        llvm::Value *param(Builder &builder) const;

    private:
        llvm::Value *_get;
        llvm::Type *_type;
    };

    class Midi : public Value {
    public:
        Midi(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos);

        Midi(MaximContext *context, Builder &allocaBuilder, SourcePos startPos, SourcePos endPos);

        static std::unique_ptr<Midi> create(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos);

        static std::unique_ptr<Midi> create(MaximContext *context, Builder &allocaBuilder, SourcePos startPos, SourcePos endPos);

        static void initialize(llvm::Module *module, MaximContext *ctx);

        llvm::Value *get() const override { return _get; }

        llvm::Value *activePtr(Builder &builder) const;

        llvm::Value *countPtr(Builder &builder) const;

        llvm::Value *eventsPtr(Builder &builder) const;

        llvm::Value *eventPtr(Builder &builder, size_t index) const;

        llvm::Value *eventPtr(Builder &builder, llvm::Value *index) const;

        llvm::Value *active(Builder &builder);

        llvm::Value *count(Builder &builder) const;

        MidiEvent eventAt(Builder &builder, size_t index) const;

        MidiEvent eventAt(Builder &builder, llvm::Value *index) const;

        void setActive(Builder &builder, bool active) const;

        void setActive(Builder &builder, llvm::Value *active) const;

        void setCount(Builder &builder, uint64_t count) const;

        void setCount(Builder &builder, llvm::Value *count) const;

        void pushEvent(Builder &builder, const MidiEvent &event, llvm::Module *module) const;

        std::unique_ptr<Value> withSource(SourcePos startPos, SourcePos endPos) const override;

        MidiType *type() const override;

        static llvm::Function *pushEventFunc(llvm::Module *module, MaximContext *ctx);

    private:
        llvm::Value *_get;
        MaximContext *_context;
    };

}
