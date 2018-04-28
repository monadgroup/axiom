#pragma once

#include "Node.h"
#include "ErrorLog.h"
#include "HardControl.h"
#include "../codegen/Scope.h"

namespace MaximAst {
    class Block;
}

namespace MaximRuntime {

    class CustomNode : public Node {
    Q_OBJECT

    public:
        explicit CustomNode(Surface *surface);

        ~CustomNode() override;

        std::string code() const { return _code; }

        void setCode(const std::string &code);

        GeneratableModuleClass *compile() override;

        void remove() override;

        const std::unique_ptr<Control> *
        begin() const override { return (const std::unique_ptr<Control> *) &_controls[0]; }

        const std::unique_ptr<Control> *
        end() const override { return (const std::unique_ptr<Control> *) &_controls[_controls.size()]; }

        const ErrorLog &errorLog() const { return _errorLog; }

        ErrorLog &errorLog() { return _errorLog; }

        MaximCodegen::ModuleClass *moduleClass() override;

    signals:

        void controlAdded(HardControl *control);

        void finishedCodegen();

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
