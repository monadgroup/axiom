#include "MidiControl.h"

#include "../MaximContext.h"
#include "../Midi.h"

using namespace MaximCodegen;

MidiControl::MidiControl(MaximContext *context) : Control(context, MaximCommon::ControlType::MIDI) {

}

std::unique_ptr<MidiControl> MidiControl::create(MaximContext *context) {
    return std::make_unique<MidiControl>(context);
}

llvm::Type* MidiControl::type(MaximContext *ctx) const {
    return llvm::PointerType::get(ctx->midiType()->get(), 0);
}

bool MidiControl::validateProperty(std::string name) {
    return name == "value";
}

void MidiControl::setProperty(Builder &b, std::string name, std::unique_ptr<Value> val, llvm::Value *ptr) {
    auto midiVal = context()->assertMidi(std::move(val));
    b.CreateStore(midiVal->get(), b.CreateLoad(ptr, "ptr"));
}

std::unique_ptr<Value> MidiControl::getProperty(Builder &b, std::string name, llvm::Value *ptr) {
    auto undefPos = SourcePos(-1, -1);
    return Midi::create(context(), b.CreateLoad(b.CreateLoad(ptr, "ptr"), "control"), undefPos, undefPos);
}
