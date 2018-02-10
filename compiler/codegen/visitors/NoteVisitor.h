#pragma once

#include <memory>

namespace MaximAst {
    class NoteExpression;
}

namespace MaximCodegen {

    class Node;

    class Value;

    std::unique_ptr<Value> visitNote(Node *node, MaximAst::NoteExpression *expr);

}
