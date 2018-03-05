#pragma once

#include "../codegen/ComposableModuleClass.h"
#include "../codegen/ComposableModuleClassMethod.h"

namespace MaximRuntime {

    class Node;

    class GeneratableModuleClass : public MaximCodegen::ComposableModuleClass {
    public:
        GeneratableModuleClass(MaximCodegen::MaximContext *ctx, llvm::Module *module, const std::string &name, const std::vector<llvm::Type*> &constructorParams = {});

        MaximCodegen::ComposableModuleClassMethod *generate() { return &_generate; }

    protected:

        void doComplete() override;

    private:

        MaximCodegen::ComposableModuleClassMethod _generate;
    };

}
