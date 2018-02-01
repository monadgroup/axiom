#pragma once

#include <string>

#include "ErrorLog.h"

namespace MaximEditor {

    class CustomNode {
    public:
        ErrorLog updateProgram(const std::string &text);
    };

}
