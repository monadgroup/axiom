#pragma once

#include <memory>

#include "../common/FormType.h"
#include "Builder.h"
#include "../SourcePos.h"

namespace MaximCodegen {

    class MaximContext;

    class Num;

    class Node;

    class Converter {
    public:
        Converter(MaximContext *context, MaximCommon::FormType toType);

        MaximCommon::FormType toType() const { return _toType; }

        virtual void generate(llvm::Module *module);

        virtual std::unique_ptr<Num> call(Node *node, std::unique_ptr<Num> value, SourcePos startPos, SourcePos endPos);

    protected:
        using FormConverter = llvm::Value *(Converter::*)(Builder &b, llvm::Value *val, llvm::Module *module);

        MaximContext *context() const { return _context; }

        std::map<MaximCommon::FormType, FormConverter> converters;

    private:
        MaximContext *_context;
        MaximCommon::FormType _toType;

        llvm::Function *createFuncForModule(llvm::Module *module);
    };

}
