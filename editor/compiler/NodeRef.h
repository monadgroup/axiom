#pragma once

#include <cstddef>

namespace MaximCompiler {

    class NodeRef {
    public:
        explicit NodeRef(void *handle);

        void *get() const { return handle; }

        void addValueSocket(size_t groupId, bool valueWritten, bool valueRead, bool isExtractor);

    private:
        void *handle;
    };

}
