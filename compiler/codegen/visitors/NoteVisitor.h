#pragma once

#include <memory>

namespace MaximAst {
    class NoteExpression;
}

namespace MaximCodegen {

    class ComposableModuleClassMethod;

    class Scope;

    class Value;

    std::unique_ptr<Value> visitNote(ComposableModuleClassMethod *method, Scope *scope, MaximAst::NoteExpression *expr);

}
