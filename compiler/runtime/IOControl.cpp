#include "IOControl.h"

#include "IONode.h"
#include "Runtime.h"

using namespace MaximRuntime;

IOControl::IOControl(IONode *node, MaximCommon::ControlType type, bool isRead, bool isWrite)
    : Control(node), _ioType(type), _type(node->runtime()->ctx()->getControl(type)),
      _isRead(isRead), _isWrite(isWrite) {
}
