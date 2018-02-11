#pragma once

#include <memory>

#include "Value.h"
#include "Builder.h"
#include "NumType.h"
#include "../common/FormType.h"

namespace llvm {
    class Value;
}

namespace MaximCodegen {

    class MaximContext;

    class Num : public Value {
    public:
        Num(MaximContext *context, float left, float right, MaximCommon::FormType form, bool active,
            SourcePos startPos, SourcePos endPos);

        Num(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos);

        static std::unique_ptr<Num>
        create(MaximContext *context, float left, float right, MaximCommon::FormType form, bool active,
               SourcePos startPos, SourcePos endPos);

        static std::unique_ptr<Num>
        create(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos);

        llvm::Value *get() const override { return _get; }

        llvm::Value *vec(Builder &builder) const;

        llvm::Value *form(Builder &builder) const;

        llvm::Value *active(Builder &builder) const;

        std::unique_ptr<Num> withVec(Builder &builder, llvm::Value *newVec, SourcePos startPos, SourcePos endPos) const;

        std::unique_ptr<Num>
        withVec(Builder &builder, float left, float right, SourcePos startPos, SourcePos endPos) const;

        std::unique_ptr<Num>
        withForm(Builder &builder, llvm::Value *newForm, SourcePos startPos, SourcePos endPos) const;

        std::unique_ptr<Num>
        withForm(Builder &builder, MaximCommon::FormType form, SourcePos startPos, SourcePos endPos) const;

        std::unique_ptr<Num>
        withActive(Builder &builder, llvm::Value *newActive, SourcePos startPos, SourcePos endPos) const;

        std::unique_ptr<Num> withActive(Builder &builder, bool active, SourcePos startPos, SourcePos endPos) const;

        std::unique_ptr<Value> withSource(SourcePos startPos, SourcePos endPos) const override;

        NumType *type() const override;

    private:
        llvm::Value *_get;
        MaximContext *_context;
    };

}
