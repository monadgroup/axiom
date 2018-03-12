#include "Num.h"

#include "MaximContext.h"

using namespace MaximCodegen;

Num::Num(MaximContext *context, Builder &allocaBuilder, float left, float right, MaximCommon::FormType form,
         bool active, SourcePos startPos, SourcePos endPos) : Value(startPos, endPos), _context(context) {
    _get = allocaBuilder.CreateAlloca(type()->get(), nullptr, "num");
    allocaBuilder.CreateStore(llvm::ConstantStruct::get(type()->get(), {
        context->constFloatVec(left, right),
        llvm::ConstantInt::get(type()->formType(), (uint64_t) form, false),
        llvm::ConstantInt::get(type()->activeType(), (uint64_t) active, false)
    }), _get);
}

Num::Num(MaximContext *context, Builder &allocaBuilder, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _context(context) {
    _get = allocaBuilder.CreateAlloca(type()->get(), nullptr, "num");
}

Num::Num(MaximContext *context, llvm::Value *clone, Builder &builder, Builder &allocaBuilder, SourcePos startPos,
         SourcePos endPos) : Value(startPos, endPos), _context(context) {
    _get = allocaBuilder.CreateAlloca(type()->get(), nullptr, "num");
    context->copyPtr(builder, clone, _get);
}

Num::Num(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _get(get), _context(context) {
}

std::unique_ptr<Num>
Num::create(MaximContext *context, Builder &allocaBuilder, float left, float right, MaximCommon::FormType form,
            bool active, SourcePos startPos, SourcePos endPos) {
    return std::make_unique<Num>(context, allocaBuilder, left, right, form, active, startPos, endPos);
}

std::unique_ptr<Num> Num::create(MaximContext *context, Builder &allocaBuilder, SourcePos startPos, SourcePos endPos) {
    return std::make_unique<Num>(context, allocaBuilder, startPos, endPos);
}

std::unique_ptr<Num> Num::create(MaximContext *context, llvm::Value *clone, Builder &builder, Builder &allocaBuilder,
                                 SourcePos startPos, SourcePos endPos) {
    return std::make_unique<Num>(context, clone, builder, allocaBuilder, startPos, endPos);
}

std::unique_ptr<Num> Num::create(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos) {
    return std::make_unique<Num>(context, get, startPos, endPos);
}

llvm::Value* Num::vecPtr(Builder &builder) const {
    return builder.CreateStructGEP(type()->get(), _get, 0, _get->getName() + ".vec.ptr");
}

llvm::Value* Num::formPtr(Builder &builder) const {
    return builder.CreateStructGEP(type()->get(), _get, 1, _get->getName() + ".form.ptr");
}

llvm::Value* Num::activePtr(Builder &builder) const {
    return builder.CreateStructGEP(type()->get(), _get, 2, _get->getName() + ".active.ptr");
}

llvm::Value* Num::vec(Builder &builder) const {
    return builder.CreateLoad(vecPtr(builder), _get->getName() + ".vec");
}

llvm::Value* Num::form(Builder &builder) const {
    return builder.CreateLoad(formPtr(builder), _get->getName() + ".form");
}

llvm::Value* Num::active(Builder &builder) const {
    return builder.CreateLoad(activePtr(builder), _get->getName() + ".active");
}

void Num::setVec(Builder &builder, float left, float right) const {
    setVec(builder, _context->constFloatVec(left, right));
}

void Num::setVec(Builder &builder, llvm::Value *value) const {
    builder.CreateStore(value, vecPtr(builder));
}

void Num::setForm(Builder &builder, MaximCommon::FormType form) const {
    setForm(builder, llvm::ConstantInt::get(type()->formType(), (uint64_t) form, false));
}

void Num::setForm(Builder &builder, llvm::Value *value) const {
    builder.CreateStore(value, formPtr(builder));
}

void Num::setActive(Builder &builder, bool active) const {
    setActive(builder, llvm::ConstantInt::get(type()->activeType(), (uint64_t) active, false));
}

void Num::setActive(Builder &builder, llvm::Value *value) const {
    builder.CreateStore(value, activePtr(builder));
}

std::unique_ptr<Value> Num::withSource(SourcePos startPos, SourcePos endPos) const {
    return Num::create(_context, _get, startPos, endPos);
}

NumType *Num::type() const {
    return _context->numType();
}
