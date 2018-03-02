#pragma once

#include "ComposableModuleClass.h"
#include "../common/FormType.h"
#include "../SourcePos.h"

namespace MaximCodegen {

    class Num;

    class Converter : public ComposableModuleClass {
    public:
        Converter(MaximContext *ctx, llvm::Module *module, MaximCommon::FormType toType);

        void generate();

        MaximCommon::FormType toType() const { return _toType; }

        std::unique_ptr<Num> call(ComposableModuleClassMethod *method, std::unique_ptr<Num> value, SourcePos startPos, SourcePos endPos);

    protected:
        using FormConverter = llvm::Value *(Converter::*)(ComposableModuleClassMethod *method, llvm::Value *val);

        std::map<MaximCommon::FormType, FormConverter> converters;

    private:
        MaximCommon::FormType _toType;

        std::unique_ptr<ComposableModuleClassMethod> _callMethod;
    };

}
