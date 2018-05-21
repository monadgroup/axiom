#pragma once

#include "common/Event.h"
#include "Node.h"
#include "Surface.h"
#include "SoftControl.h"

namespace MaximRuntime {

    class Control;

    class GeneratableModuleClass;

    class GroupNode : public Node, public AxiomCommon::Hookable {
    public:
        AxiomCommon::Event<SoftControl *> controlAdded;

        explicit GroupNode(Surface *surface);

        GeneratableModuleClass *compile() override;

        std::vector<Control*> controls() const override;

        Surface *subsurface() { return &_subsurface; }

        SoftControl *forwardControl(Control *control);

        void pullMethods(MaximCodegen::ModuleClassMethod *getterMethod,
                         MaximCodegen::ModuleClassMethod *destroyMethod) override;

        void *updateCurrentPtr(void *parentCtx) override;

        void saveValue() override;

        void restoreValue() override;

        MaximCodegen::ModuleClass *moduleClass() override;

    private:

        Surface _subsurface;

        std::vector<std::unique_ptr<SoftControl>> _controls;

        void removeControl(SoftControl *control);
    };

}
