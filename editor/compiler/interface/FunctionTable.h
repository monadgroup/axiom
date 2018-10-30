#pragma once

#include <QtCore/QString>

namespace MaximCompiler {

    class FunctionTable {
    public:
        static size_t size();
        static QString find(size_t index);
    };
}
