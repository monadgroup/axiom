#include "ValueReader.h"

#include "../codegen/MaximContext.h"

using namespace MaximRuntime;

ValueReader::ValueReader(MaximCodegen::MaximContext *context) : _context(context) {
    auto numLayout = context->numType()->layout();
    numValOffset = numLayout->getElementOffset(0);
    numFormOffset = numLayout->getElementOffset(1);
    numActiveOffset = numLayout->getElementOffset(2);
}

NumValue ValueReader::readNum(void *ptr) {
    auto bytePtr = (uint8_t*) ptr;
    auto vecPtr = (float*)(bytePtr + numValOffset);
    return {
        *(vecPtr + 0),
        *(vecPtr + 1),
        (MaximCommon::FormType) *(bytePtr + numFormOffset),
        (bool) *(bytePtr + numActiveOffset)
    };
}
