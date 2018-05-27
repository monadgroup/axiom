#pragma once

#include "common/Event.h"
#include "Node.h"
#include "ErrorLog.h"
#include "HardControl.h"
#include "../codegen/Scope.h"

namespace MaximAst {
    class Block;
}

namespace MaximRuntime {

    class CustomNode : public Node {
    public:
        AxiomCommon::Event<HardControl *> controlAdded;
        AxiomCommon::Event<> finishedCodegen;

        explicit CustomNode(Surface *surface);

        ~CustomNode() override;

        std::string code() const { return _code; }

        void setCode(const std::string &code);

        GeneratableModuleClass *compile() override;

        void remove() override;

        std::vector<Control*> controls() const override;

        const ErrorLog &errorLog() const { return _errorLog; }

        ErrorLog &errorLog() { return _errorLog; }

        MaximCodegen::ModuleClass *moduleClass() override;

        void pullMethods(MaximCodegen::ModuleClassMethod *getterMethod, MaximCodegen::ModuleClassMethod *destroyMethod) override;

    private:

        std::unique_ptr<GeneratableModuleClass> _moduleClass;

        MaximCodegen::Scope _scope;

        // set to not-blank so when we compile in the constructor, it won't ignore the same value
        std::string _code = "<>";

        std::unique_ptr<MaximAst::Block> _ast;

        ErrorLog _errorLog;

        std::vector<std::unique_ptr<HardControl>> _controls;

        void updateControls();
    };

}
