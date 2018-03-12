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
        Num(MaximContext *context, Builder &allocaBuilder, float left, float right, MaximCommon::FormType form,
            bool active, SourcePos startPos, SourcePos endPos);

        Num(MaximContext *context, Builder &allocaBuilder, SourcePos startPos, SourcePos endPos);

        Num(MaximContext *context, llvm::Value *clone, Builder &builder, Builder &allocaBuilder, SourcePos startPos,
            SourcePos endPos);

        Num(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos);

        static std::unique_ptr<Num>
        create(MaximContext *context, Builder &allocaBuilder, float left, float right, MaximCommon::FormType form,
               bool active, SourcePos startPos = SourcePos(-1, -1), SourcePos endPos = SourcePos(-1, -1));

        static std::unique_ptr<Num>
        create(MaximContext *context, Builder &allocaBuilder, SourcePos startPos = SourcePos(-1, -1), SourcePos endPos = SourcePos(-1, -1));

        static std::unique_ptr<Num>
        create(MaximContext *context, llvm::Value *clone, Builder &builder, Builder &allocaBuilder, SourcePos startPos = SourcePos(-1, -1),
               SourcePos endPos = SourcePos(-1, -1));

        static std::unique_ptr<Num>
        create(MaximContext *context, llvm::Value *get, SourcePos startPos = SourcePos(-1, -1), SourcePos endPos = SourcePos(-1, -1));

        llvm::Value *get() const override { return _get; }

        llvm::Value *vecPtr(Builder &builder) const;

        llvm::Value *formPtr(Builder &builder) const;

        llvm::Value *activePtr(Builder &builder) const;

        llvm::Value *vec(Builder &builder) const;

        llvm::Value *form(Builder &builder) const;

        llvm::Value *active(Builder &builder) const;

        void setVec(Builder &builder, float left, float right) const;

        void setVec(Builder &builder, llvm::Value *value) const;

        void setForm(Builder &builder, MaximCommon::FormType form) const;

        void setForm(Builder &builder, llvm::Value *value) const;

        void setActive(Builder &builder, bool active) const;

        void setActive(Builder &builder, llvm::Value *value) const;

        std::unique_ptr<Value> withSource(SourcePos startPos, SourcePos endPos) const override;

        NumType *type() const override;

    private:
        llvm::Value *_get;
        MaximContext *_context;
    };

}
