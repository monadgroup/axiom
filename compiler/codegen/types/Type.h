#pragma once

namespace llvm {
    class Type;
}

namespace MaximCodegen {

    class Context;

    class Type {
    public:
        explicit Type(Context *context);

        Context *context() const { return _context; }
        virtual llvm::Type *llType() const = 0;

    private:
        Context *_context;
    };

}
