#pragma once

#include "../Converter.h"

namespace MaximCodegen {

    class LinearConverter : public Converter {
    public:
        explicit LinearConverter(MaximContext *context);

        static std::unique_ptr<LinearConverter> create(MaximContext *context);

        void generate(llvm::Module *module) override;

        std::unique_ptr<Num>
        call(Node *node, std::unique_ptr<Num> value, SourcePos startPos, SourcePos endPos) override;
    };

}
