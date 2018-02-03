#include "Scope.h"

#include <cassert>

using namespace MaximCodegen;

Value *Scope::findValue(std::string name) {
    // todo
    assert(false);
    throw;
}

void Scope::setValue(std::string name, std::unique_ptr<Value> value) {
    // todo
    assert(false);
    throw;
}

Value *Scope::findControl(std::string name, MaximAst::ControlExpression::Type type, std::string property) {
    // todo
    assert(false);
    throw;
}

void Scope::setControl(std::string name, MaximAst::ControlExpression::Type type, std::string property,
                       std::unique_ptr<Value> value) {
    // todo
    assert(false);
    throw;
}
