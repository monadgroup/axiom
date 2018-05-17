#pragma once

#include <optional>

#include "common/Event.h"
#include "Node.h"

namespace MaximRuntime {
    class CustomNode;

    class Control;
}

namespace AxiomModel {

    class CustomNode : public Node {
    public:
        static constexpr float minPanelHeight = 40;

        AxiomCommon::Event<const QString &> codeChanged;
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
                    bool selected, QString name, const QUuid &controlsUuid, ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        const QString &code() const { return _code; }

        void setCode(const QString &code);

        bool isPanelOpen() const { return _isPanelOpen; }

        void setPanelOpen(bool panelOpen);

        float panelHeight() const { return _panelHeight; }

        void setPanelHeight(float panelHeight);

        void createAndAttachRuntime(MaximRuntime::Surface *parent) override;

        void attachRuntime(MaximRuntime::CustomNode *runtime);

        void detachRuntime();

    private:
        QString _code;
        bool _isPanelOpen;
        float _panelHeight;

        std::optional<MaximRuntime::CustomNode *> _runtime;

        void runtimeAddedControl(MaximRuntime::Control *control);

        void runtimeFinishedCodegen();
    };

}
