#include "ControlInitializer.h"

#include "Frontend.h"

using namespace MaximCompiler;

ControlInitializer::ControlInitializer(void *handle)
    : OwnedObject(handle, &MaximFrontend::maxim_destroy_control_initializer) {}

ControlInitializer ControlInitializer::none() {
    return ControlInitializer(MaximFrontend::maxim_control_initializer_none());
}

ControlInitializer ControlInitializer::graph(uint8_t curveCount, size_t startValuesCount, const double *startValues,
                                             size_t endPositionsCount, const double *endPositions, size_t tensionsCount,
                                             const double *tensions, size_t statesCount, const uint8_t *states) {
    return ControlInitializer(
        MaximFrontend::maxim_control_initializer_graph(curveCount, startValuesCount, startValues, endPositionsCount,
                                                       endPositions, tensionsCount, tensions, statesCount, states));
}
