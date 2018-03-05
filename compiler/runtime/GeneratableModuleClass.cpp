#include "GeneratableModuleClass.h"

using namespace MaximRuntime;

GeneratableModuleClass::GeneratableModuleClass(MaximCodegen::MaximContext *ctx, llvm::Module *module,
                                               const std::string &name,
                                               const std::vector<llvm::Type*> &constructorParams)
    : ComposableModuleClass(ctx, module, name, constructorParams), _generate(this, "generate") {

}
