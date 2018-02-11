#pragma once

#include "Instantiable.h"

namespace MaximCodegen {

    class Function;

    class FunctionCall : public Instantiable {
        FunctionCall(Function *func, std::vector<std::unique_ptr<Value>> args);

        //llvm::Constant *instantiate() override;

    protected:
        Function *func;
        std::vector<std::unique_ptr<Value>> params;
    };

}
