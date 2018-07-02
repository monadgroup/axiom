#pragma once

#include <optional>

#include "common/Event.h"
#include "Node.h"

namespace AxiomModel {

    class Control;

    class CompositeAction;

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
                    bool selected, QString name, const QUuid &controlsUuid, ReferenceMapper *ref, ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        const QString &code() const { return _code; }

        void setCode(const QString &code);

        void doSetCodeAction(QString beforeCode, QString afterCode);

        bool isPanelOpen() const { return _isPanelOpen; }

        void setPanelOpen(bool panelOpen);

        float panelHeight() const { return _panelHeight; }

        void setPanelHeight(float panelHeight);

    private:
        QString _code;
        bool _isPanelOpen;
        float _panelHeight;
        CompositeAction *changeCodeAction = nullptr;
    };

}
