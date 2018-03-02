#include "LinearConverter.h"

#include "../Num.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

LinearConverter::LinearConverter(MaximContext *ctx, llvm::Module *module)
    : Converter(ctx, module, MaximCommon::FormType::LINEAR) {

}

std::unique_ptr<LinearConverter> LinearConverter::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<LinearConverter>(ctx, module);
}
