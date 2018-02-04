#include "MidiValue.h"

#include "../Context.h"
#include "../Function.h"

using namespace MaximCodegen;

MidiValue::MidiValue(bool isConst, llvm::Value *type, llvm::Value *channel, llvm::Value *note, llvm::Value *param,
                     llvm::Value *time, Context *context, Function *function) : Value(isConst), _context(context) {
    _value = function->initBuilder().CreateAlloca(this->type(), nullptr, "midi_val");

    auto cb = function->codeBuilder();
    cb.CreateStore(type, typePtr(cb));
    cb.CreateStore(channel, channelPtr(cb));
    cb.CreateStore(note, notePtr(cb));
    cb.CreateStore(param, paramPtr(cb));
    cb.CreateStore(time, timePtr(cb));
}

MidiValue::MidiValue(bool isConst, llvm::Value *value, Context *context)
        : Value(isConst), _value(value), _context(context) {

}

llvm::StructType *MidiValue::type() const {
    return _context->getStructType(Context::Type::MIDI);
}

llvm::Value *MidiValue::typePtr(llvm::IRBuilder<> &builder) const {
    return _context->getPtr(_value, 0, builder);
}

llvm::Value *MidiValue::channelPtr(llvm::IRBuilder<> &builder) const {
    return _context->getPtr(_value, 1, builder);
}

llvm::Value *MidiValue::notePtr(llvm::IRBuilder<> &builder) const {
    return _context->getPtr(_value, 2, builder);
}

llvm::Value *MidiValue::paramPtr(llvm::IRBuilder<> &builder) const {
    return _context->getPtr(_value, 3, builder);
}

llvm::Value *MidiValue::timePtr(llvm::IRBuilder<> &builder) const {
    return _context->getPtr(_value, 4, builder);
}

std::unique_ptr<Value> MidiValue::clone() const {
    return std::make_unique<MidiValue>(isConst(), _value, _context);
}
