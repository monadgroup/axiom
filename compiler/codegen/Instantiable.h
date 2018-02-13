#pragma once

namespace llvm {
    class Constant;
    class Type;
}

namespace MaximCodegen {

    class Instantiable {
    public:
        virtual llvm::Constant *instantiate() = 0;
        virtual llvm::Type *type() const = 0;
    };

}
