#include "Num.h"

#include <llvm/IR/Constants.h>

#include "MaximContext.h"

using namespace MaximCodegen;

Num::Num(MaximContext *context, float left, float right, MaximCommon::FormType form, bool active,
         SourcePos startPos, SourcePos endPos) : Value(startPos, endPos), _context(context) {
    _get = llvm::ConstantStruct::get(type()->get(), {
        llvm::ConstantVector::get({context->constFloat(left), context->constFloat(right)}),
        llvm::ConstantInt::get(type()->formType(), (uint64_t) form, false),
        llvm::ConstantInt::get(type()->activeType(), (uint64_t) active, false)
    });
    _get->setName("num");
}

Num::Num(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _get(get), _context(context) {
}

std::unique_ptr<Num>
Num::create(MaximContext *context, float left, float right, MaximCommon::FormType form, bool active,
            SourcePos startPos, SourcePos endPos) {
    return std::make_unique<Num>(context, left, right, form, active, startPos, endPos);
}

std::unique_ptr<Num> Num::create(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos) {
    return std::make_unique<Num>(context, get, startPos, endPos);
}

llvm::Value *Num::vec(Builder &builder) const {
    return builder.CreateExtractValue(_get, {0}, _get->getName() + ".vec");
}

llvm::Value *Num::form(Builder &builder) const {
    return builder.CreateExtractValue(_get, {1}, _get->getName() + ".form");
}

llvm::Value *Num::active(Builder &builder) const {
    return builder.CreateExtractValue(_get, {2}, _get->getName() + ".active");
}

std::unique_ptr<Num> Num::withVec(Builder &builder, llvm::Value *newVec, SourcePos startPos, SourcePos endPos) const {
    return Num::create(
        _context,
        builder.CreateInsertValue(_get, newVec, {0}, _get->getName() + ".new_vec"),
        startPos, endPos
    );
}

std::unique_ptr<Num> Num::withVec(Builder &builder, float left, float right, SourcePos startPos,
                                  SourcePos endPos) const {
    return withVec(
        builder, llvm::ConstantVector::get({_context->constFloat(left), _context->constFloat(right)}),
        startPos, endPos
    );
}

std::unique_ptr<Num> Num::withForm(Builder &builder, llvm::Value *newForm, SourcePos startPos,
                                   SourcePos endPos) const {
    return Num::create(
        _context,
        builder.CreateInsertValue(_get, newForm, {1}, _get->getName() + ".new_form"),
        startPos, endPos
    );
}

std::unique_ptr<Num> Num::withForm(Builder &builder, MaximCommon::FormType form, SourcePos startPos,
                                   SourcePos endPos) const {
    return withForm(
        builder, llvm::ConstantInt::get(type()->formType(), (uint64_t) form, false),
        startPos, endPos
    );
}

std::unique_ptr<Num> Num::withActive(Builder &builder, llvm::Value *newActive, SourcePos startPos,
                                     SourcePos endPos) const {
    return Num::create(
        _context,
        builder.CreateInsertValue(_get, newActive, {2}, _get->getName() + ".new_active"),
        startPos, endPos
    );
}

std::unique_ptr<Num> Num::withActive(Builder &builder, bool active, SourcePos startPos, SourcePos endPos) const {
    return withActive(
        builder, llvm::ConstantInt::get(type()->activeType(), (uint64_t) active, false),
        startPos, endPos
    );
}

std::unique_ptr<Value> Num::withSource(SourcePos startPos, SourcePos endPos) const {
    return Num::create(_context, _get, startPos, endPos);
}

NumType *Num::type() const {
    return _context->numType();
}
