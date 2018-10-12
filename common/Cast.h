#pragma once

namespace AxiomCommon {

    template<class Output, class Input>
    Output dynamicCast(Input input) {
        return dynamic_cast<Output>(input);
    }

    template<class Output, class Input>
    Output staticCast(Input input) {
        return static_cast<Output>(input);
    }

    template<class Output, class Input>
    Output reinterpretCast(Input input) {
        return reinterpret_cast<Output>(input);
    }
}
