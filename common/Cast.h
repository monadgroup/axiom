#pragma once

namespace AxiomCommon {

    template<class Output, class Input>
    Output wrapDynamicCast(Input input) {
        return dynamic_cast<Output>(input);
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
