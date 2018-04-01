#pragma once

#include <QtCore/QObject>

#include "Node.h"
#include "Surface.h"
#include "SoftControl.h"

namespace MaximRuntime {

    class Control;

    class GeneratableModuleClass;

    class GroupNode : public QObject, public Node {
        Q_OBJECT

    public:
        explicit GroupNode(Surface *surface);

        GeneratableModuleClass *compile() override;

        const std::unique_ptr<Control> *begin() const override { return (const std::unique_ptr<Control> *) _controls.begin().base(); }

        const std::unique_ptr<Control> *end() const override { return (const std::unique_ptr<Control> *) _controls.end().base(); }

        Surface *subsurface() { return &_subsurface; }

        void forwardControl(Control *control);

        void pullGetterMethod(MaximCodegen::ComposableModuleClassMethod *method = nullptr) override;

        void *updateCurrentPtr(void *parentCtx) override;

        void saveValue() override;

        void restoreValue() override;

        void setRestoreValue(SaveValue &value) override;

        MaximCodegen::ModuleClass *moduleClass() override;

    signals:

        void controlAdded(SoftControl *control);

    private:

        Surface _subsurface;

        std::vector<std::unique_ptr<SoftControl>> _controls;
    };

}
