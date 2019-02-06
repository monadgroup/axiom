#pragma once

#include <optional>

namespace AxiomCommon {

    template<class Output, class Input>
    std::optional<Output> wrapDynamicCast(Input input) {
        if (auto cast = dynamic_cast<Output>(input)) {
            return cast;
        }
        return std::nullopt;
    }

    template<class Output, class Input>
    Output wrapStaticCast(Input input) {
        return static_cast<Output>(input);
    }

    template<class Output, class Input>
    Output wrapReinterpretCast(Input input) {
        return reinterpret_cast<Output>(input);
    }
}
