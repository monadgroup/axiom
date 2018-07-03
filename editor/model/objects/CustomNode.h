#pragma once

#include <optional>

#include "common/Event.h"
#include "Node.h"
#include "editor/compiler/interface/Block.h"

namespace AxiomModel {

    class Control;

    class CompositeAction;

    class CustomNode : public Node {
    public:
        static constexpr float minPanelHeight = 40;

        AxiomCommon::Event<const QString &> codeChanged;
        AxiomCommon::Event<const QString &, MaximFrontend::SourceRange> codeCompileError;
        AxiomCommon::Event<> codeCompileSuccess;
        AxiomCommon::Event<bool> panelOpenChanged;
        AxiomCommon::Event<float> beforePanelHeightChanged;
        AxiomCommon::Event<float> panelHeightChanged;

        CustomNode(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                   const QUuid &controlsUuid, QString code, bool panelOpen, float panelHeight, ModelRoot *root);

        static std::unique_ptr<CustomNode>
        create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
               const QUuid &controlsUuid, QString code, bool panelOpen, float panelHeight, ModelRoot *root);

        static std::unique_ptr<CustomNode>
        deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                    bool selected, QString name, const QUuid &controlsUuid, ReferenceMapper *ref, ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        const QString &code() const { return _code; }

        void setCode(const QString &code);

        void doSetCodeAction(QString beforeCode, QString afterCode);

        bool isPanelOpen() const { return _isPanelOpen; }

        void setPanelOpen(bool panelOpen);

        float panelHeight() const { return _panelHeight; }

        void setPanelHeight(float panelHeight);

        uint64_t getRuntimeId(MaximCompiler::Runtime &runtime);

        void attachRuntime(MaximCompiler::Runtime *runtime) override;

        std::optional<MaximCompiler::Block> compiledBlock() const;

    private:
        QString _code;
        bool _isPanelOpen;
        float _panelHeight;
        uint64_t runtimeId = 0;
        MaximCompiler::Runtime *_runtime = nullptr;
        std::optional<MaximCompiler::Block> _compiledBlock;

        void buildCode();
        void updateControls(CompositeAction *actionGroup);
    };

}
