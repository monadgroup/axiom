#include "ComposableModuleClassMethod.h"

#include "ComposableModuleClass.h"

using namespace MaximCodegen;

ComposableModuleClassMethod::ComposableModuleClassMethod(ComposableModuleClass *moduleClass, std::string name,
                                                         llvm::Type *returnType,
                                                         std::vector<llvm::Type *> paramTypes)
    : ModuleClassMethod(moduleClass, std::move(name), returnType, std::move(paramTypes)),
      _composableModuleClass(moduleClass) {

}

llvm::Value* ComposableModuleClassMethod::getEntryPointer(size_t index, const llvm::Twine &name) {
    return moduleClass()->getEntryPointer(builder(), index, contextPtr(), name);
}

llvm::Value* ComposableModuleClassMethod::callInto(size_t index, const std::vector<llvm::Value *> &args,
                                                   const ModuleClassMethod *internalMethod,
                                                   const llvm::Twine &resultName) {
    auto indexStr = std::to_string(index);
    auto itemPtr = getEntryPointer(index, "intoptr." + indexStr);
    return internalMethod->call(builder(), args, itemPtr, moduleClass()->module(), resultName);
}
