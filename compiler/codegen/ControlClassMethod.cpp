#include "ControlClassMethod.h"

#include "ControlField.h"

using namespace MaximCodegen;

ControlClassMethod::ControlClassMethod(ModuleClass *parentClass, std::string name, llvm::Type *returnType,
                                                 std::vector<llvm::Type *> paramTypes)
    : ModuleClassMethod(parentClass, std::move(name), returnType, std::move(paramTypes)) {
    auto ptr = contextPtr();
    _groupPtr = builder().CreateLoad(builder().CreateStructGEP(ptr->getType()->getPointerElementType(), ptr, 0, "controlgroup"));
    _storagePtr = builder().CreateStructGEP(ptr->getType()->getPointerElementType(), ptr, 1, "controlstorage");
}
