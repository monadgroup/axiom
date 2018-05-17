#pragma once

#include "common/Event.h"
#include "Node.h"
#include "Surface.h"
#include "SoftControl.h"

namespace MaximRuntime {

    class Control;

    class GeneratableModuleClass;

    class GroupNode : public Node {
    public:
        AxiomCommon::Event<SoftControl *> controlAdded;

        explicit GroupNode(Surface *surface);

        GeneratableModuleClass *compile() override;

        const std::unique_ptr<Control> *
        begin() const override { return (const std::unique_ptr<Control> *) &_controls[0]; }

        const std::unique_ptr<Control> *
        end() const override { return (const std::unique_ptr<Control> *) &_controls[_controls.size()]; }

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
    };

}
