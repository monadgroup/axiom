#include "NoneConverter.h"

#include "../Num.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

NoneConverter::NoneConverter(MaximCodegen::MaximContext *ctx, llvm::Module *module)
    : Converter(ctx, module, MaximCommon::FormType::NONE) {
}

std::unique_ptr<NoneConverter> NoneConverter::create(MaximCodegen::MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<NoneConverter>(ctx, module);
}
