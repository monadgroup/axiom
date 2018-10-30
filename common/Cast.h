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
    std::optional<Output> wrapStaticCast(Input input) {
        if (auto cast = static_cast<Output>(input)) {
            return cast;
        }
        return std::nullopt;
    }

    template<class Output, class Input>
    std::optional<Output> wrapReinterpretCast(Input input) {
        if (auto cast = reinterpret_cast<Output>(input)) {
            return cast;
        }
        return std::nullopt;
    }
}
