#pragma once

#include <optional>

#include "Node.h"
#include "common/Event.h"
#include "editor/compiler/interface/Block.h"

namespace AxiomModel {

    class Control;

    class SetCodeAction;

    struct CustomNodeError {
        QString message;
        MaximFrontend::SourceRange sourceRange;

        CustomNodeError(QString message, MaximFrontend::SourceRange sourceRange)
            : message(std::move(message)), sourceRange(sourceRange) {}
    };

    class CustomNode : public Node {
    public:
        static constexpr float minPanelHeight = 40;

        AxiomCommon::Event<const QString &> codeChanged;
        AxiomCommon::Event<const CustomNodeError &> codeCompileError;
        AxiomCommon::Event<> codeCompileSuccess;
        AxiomCommon::Event<bool> panelOpenChanged;
        AxiomCommon::Event<float> beforePanelHeightChanged;
        AxiomCommon::Event<float> panelHeightChanged;

        CustomNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                   const QUuid &controlsUuid, QString code, bool panelOpen, float panelHeight, ModelRoot *root);

        static std::unique_ptr<CustomNode> create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                  bool selected, QString name, const QUuid &controlsUuid, QString code,
                                                  bool panelOpen, float panelHeight, ModelRoot *root);

        const QString &code() const { return _code; }

        QString debugName() override;

        void setCode(const QString &code);

        void promoteStaging();

        void doSetCodeAction(QString beforeCode, QString afterCode);

        bool isPanelOpen() const { return _isPanelOpen; }

        void setPanelOpen(bool panelOpen);

        float panelHeight() const { return _panelHeight; }

        void setPanelHeight(float panelHeight);

        uint64_t getRuntimeId() { return runtimeId; }

        void attachRuntime(MaximCompiler::Runtime *runtime, MaximCompiler::Transaction *transaction) override;

        void updateRuntimePointers(MaximCompiler::Runtime *runtime, void *surfacePtr) override;

        bool hasValidBlock() const { return static_cast<bool>(_compiledBlock); }

        const std::optional<CustomNodeError> &compileError() const;

        void build(MaximCompiler::Transaction *transaction) override;

    private:
        QString _code;
        bool _isPanelOpen;
        float _panelHeight;
        uint64_t runtimeId = 0;

        // "compiledBlock" is what's currently being used, "stagingBlock" is the one being typed in, and could be NULL
        // to signify a compile error (in which case we can't promote it to compiled)
        std::optional<MaximCompiler::Block> _compiledBlock;
        std::optional<MaximCompiler::Block> _stagingBlock;
        std::optional<CustomNodeError> _compileError;

        void updateControls(SetCodeAction *action);

        void surfaceControlAdded(Control *control);

        void buildCode();
    };
}
