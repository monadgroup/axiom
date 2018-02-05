#include "Scope.h"

#include "Context.h"
#include "Control.h"
#include "values/Value.h"

using namespace MaximCodegen;

Scope::Scope(Context *context) : _context(context) {

}

Value *Scope::findValue(std::string name) {
    auto pair = values.find(name);
    if (pair == values.end()) return nullptr;
    return pair->second.get();
}

void Scope::setValue(std::string name, std::unique_ptr<Value> value) {
    value->value()->setName(name);

    auto pair = values.find(name);
    if (pair == values.end()) values.emplace(name, std::move(value));
    else pair->second = std::move(value);
}

Control* Scope::getControl(std::string name, MaximAst::ControlExpression::Type type) {
    ControlKey key = { name, type };

    auto pair = controls.find(key);
    if (pair == controls.end()) {
        auto newControl = std::make_unique<Control>(_context->getControlDecl(type));
        auto controlPtr = newControl.get();
        controls.emplace(key, std::move(newControl));
        return controlPtr;
    }

    return pair->second.get();
}

bool ControlKey::operator==(const ControlKey &other) const {
    return name == other.name && type == other.type;
}
