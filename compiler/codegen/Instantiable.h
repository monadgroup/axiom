#pragma once

namespace llvm {
    class Constant;
}

namespace MaximCodegen {

    class Instantiable {
    public:
        virtual llvm::Constant *instantiate() = 0;
    };

}
