#include "Value.h"

using namespace MaximCodegen;

Value::Value(SourcePos startPos, SourcePos endPos) : startPos(startPos), endPos(endPos) {

}

std::unique_ptr<Value> Value::clone() const {
    return withSource(startPos, endPos);
}
