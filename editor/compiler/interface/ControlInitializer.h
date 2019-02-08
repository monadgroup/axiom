#pragma once

#include <cstddef>
#include <cstdint>

#include "OwnedObject.h"

namespace MaximCompiler {

    class ControlInitializer : public OwnedObject {
    public:
        static ControlInitializer none();

        static ControlInitializer graph(uint8_t curveCount, size_t startValuesCount, const double *startValues,
                                        size_t endPositionsCount, const double *endPositions, size_t tensionsCount,
                                        const double *tensions, size_t statesCount, const uint8_t *states);

    private:
        explicit ControlInitializer(void *handle);
    };
}
