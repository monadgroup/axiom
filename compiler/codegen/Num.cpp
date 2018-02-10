#include "Num.h"

#include <llvm/IR/Constants.h>

#include "MaximContext.h"

using namespace MaximCodegen;

Num::Num(MaximContext *context, float left, float right, MaximCommon::FormType type, bool active,
         SourcePos startPos, SourcePos endPos) : Value(startPos, endPos), _context(context) {
    _get = llvm::ConstantStruct::get(context->numType(), {
        llvm::ConstantVector::get({context->constFloat(left), context->constFloat(right)}),
        llvm::ConstantInt::get(context->numFormType(), (uint64_t) type, false),
        llvm::ConstantInt::get(context->numActiveType(), (uint64_t) active, false)
    });
}

Num::Num(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _get(get), _context(context) {
}

std::unique_ptr<Num>
Num::create(MaximContext *context, float left, float right, MaximCommon::FormType type, bool active,
            SourcePos startPos, SourcePos endPos) {
    return std::make_unique<Num>(context, left, right, type, active, startPos, endPos);
}

std::unique_ptr<Num> Num::create(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos) {
    return std::make_unique<Num>(context, get, startPos, endPos);
}

llvm::Value *Num::vec(Builder &builder) const {
    return builder.CreateExtractValue(_get, {0}, "num.vec");
}

llvm::Value *Num::form(Builder &builder) const {
    return builder.CreateExtractValue(_get, {1}, "num.form");
}

llvm::Value *Num::active(Builder &builder) const {
    return builder.CreateExtractValue(_get, {2}, "num.active");
}

std::unique_ptr<Num> Num::withVec(Builder &builder, llvm::Value *newVec, SourcePos startPos, SourcePos endPos) const {
    return Num::create(
        _context,
        builder.CreateInsertValue(_get, newVec, {0}, "num.new_vec"),
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
        builder.CreateInsertValue(_get, newForm, {1}, "num.new_form"),
        startPos, endPos
    );
}

std::unique_ptr<Num> Num::withForm(Builder &builder, MaximCommon::FormType type, SourcePos startPos,
                                   SourcePos endPos) const {
    return withForm(
        builder, llvm::ConstantInt::get(_context->numFormType(), (uint64_t) type, false),
        startPos, endPos
    );
}

std::unique_ptr<Num> Num::withActive(Builder &builder, llvm::Value *newActive, SourcePos startPos,
                                     SourcePos endPos) const {
    return Num::create(
        _context,
        builder.CreateInsertValue(_get, newActive, {2}, "num.new_active"),
        startPos, endPos
    );
}

std::unique_ptr<Num> Num::withActive(Builder &builder, bool active, SourcePos startPos, SourcePos endPos) const {
    return withForm(
        builder, llvm::ConstantInt::get(_context->numActiveType(), (uint64_t) active, false),
        startPos, endPos
    );
}

std::unique_ptr<Value> Num::withSource(SourcePos startPos, SourcePos endPos) const {
    return Num::create(_context, _get, startPos, endPos);
}
