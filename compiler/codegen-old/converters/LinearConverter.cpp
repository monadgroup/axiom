#include "LinearConverter.h"

#include "../Num.h"
#include "../Node.h"

using namespace MaximCodegen;

LinearConverter::LinearConverter(MaximContext *context) : Converter(context, MaximCommon::FormType::LINEAR) {

}

std::unique_ptr<LinearConverter> LinearConverter::create(MaximContext *context) {
    return std::make_unique<LinearConverter>(context);
}

void LinearConverter::generate(llvm::Module *module) {
    // linear converter is a direct mapping, so we override this to avoid generating a function we don't use
}

std::unique_ptr<Num> LinearConverter::call(Node *node, std::unique_ptr<Num> value, SourcePos startPos,
                                           SourcePos endPos) {
    return value->withForm(node->builder(), MaximCommon::FormType::LINEAR, startPos, endPos);
}
