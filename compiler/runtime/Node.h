#pragma once

#include <set>

#include "common/Event.h"
#include "ModuleRuntimeUnit.h"

namespace MaximRuntime {

    class GeneratableModuleClass;

    class Surface;

    class Control;

    class Node : public ModuleRuntimeUnit {
    public:
        AxiomCommon::Event<bool> extractedChanged;

        explicit Node(Surface *surface);

        virtual GeneratableModuleClass *compile() = 0;

        virtual void remove();

        Surface *surface() const { return _surface; }

        virtual std::vector<Control*> controls() const = 0;

        void scheduleCompile();

        virtual void scheduleChildUpdate();

        bool needsCompile() const { return _needsCompile; }

        bool extracted() const { return _extracted; }

        void setExtracted(bool extracted);

    protected:

        bool _needsCompile = false;

    private:

        Surface *_surface;

        bool _extracted = false;
    };

}
