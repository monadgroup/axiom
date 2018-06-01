#pragma once

#include <functional>

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

        std::unique_ptr<Num>
        call(ComposableModuleClassMethod *method, std::unique_ptr<Num> value, SourcePos startPos, SourcePos endPos);

        const std::string &wrapFunctionName() const { return _wrapFunctionName; }

    protected:
        using FormConverter = std::function<llvm::Value *(ComposableModuleClassMethod *, llvm::Value *)>;

        std::map<MaximCommon::FormType, FormConverter> converters;

    private:
        MaximCommon::FormType _toType;

        std::unique_ptr<ComposableModuleClassMethod> _callMethod;
        llvm::Function *_wrapFunction;
        std::string _wrapFunctionName;

        void generateCall();

        void generateWrapper();
    };

}
