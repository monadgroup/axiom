#include "ValueOperator.h"

#include "../codegen/MaximContext.h"

using namespace MaximRuntime;

ValueOperator::ValueOperator(MaximCodegen::MaximContext *context) : _context(context) {
    auto numLayout = context->numType()->layout();
    numValOffset = numLayout->getElementOffset(0);
    numFormOffset = numLayout->getElementOffset(1);
    numActiveOffset = numLayout->getElementOffset(2);
}

NumValue ValueOperator::readNum(void *ptr) {
    auto bytePtr = (uint8_t*) ptr;
    auto vecPtr = (float*)(bytePtr + numValOffset);
    return {
        *(vecPtr + 0),
        *(vecPtr + 1),
        (MaximCommon::FormType) *(bytePtr + numFormOffset),
        (bool) *(bytePtr + numActiveOffset)
    };
}

void ValueOperator::writeNum(void *ptr, NumValue value) {
    auto bytePtr = (uint8_t*) ptr;
    auto vecPtr = (float*)(bytePtr + numValOffset);
    *(vecPtr + 0) = value.left;
    *(vecPtr + 1) = value.right;
    *(bytePtr + numFormOffset) = (uint8_t) value.form;
    *(bytePtr + numActiveOffset) = (uint8_t) value.active;
}
