#pragma once

namespace MaximRuntime {
    class Runtime;
}

namespace AxiomStandalone {

    void startupAudio(MaximRuntime::Runtime *runtime);

    void shutdownAudio();

}
