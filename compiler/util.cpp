#include "util.h"

#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>

std::string AxiomUtil::debugLLType(llvm::Type *type) {
    std::string type_str;
    llvm::raw_string_ostream rso(type_str);
    type->print(rso);
    return rso.str();
}
