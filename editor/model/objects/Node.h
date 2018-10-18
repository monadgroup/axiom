#pragma once

#include "../ModelObject.h"
#include "../grid/GridItem.h"
#include "common/Event.h"
#include "common/Promise.h"
#include "editor/compiler/interface/Transaction.h"

namespace MaximCompiler {
    class Runtime;
    class Transaction;
}

namespace AxiomModel {

    class NodeSurface;

    class ControlSurface;

    struct NodeCompileMeta {
        size_t mirIndex;

        explicit NodeCompileMeta(size_t mirIndex) : mirIndex(mirIndex) {}
    };

    class Node : public GridItem, public ModelObject {
    public:
        enum class NodeType { CUSTOM_NODE, GROUP_NODE, PORTAL_NODE };

        AxiomCommon::Event<const QString &> nameChanged;
        AxiomCommon::Event<bool> extractedChanged;
        AxiomCommon::Event<bool> activeChanged;

        Node(NodeType nodeType, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
             QString name, const QUuid &controlsUuid, ModelRoot *root);

        NodeSurface *surface() const { return _surface; }

        AxiomCommon::Promise<ControlSurface *> &controls() { return *_controls; }

        const AxiomCommon::Promise<ControlSurface *> &controls() const { return *_controls; }

        NodeType nodeType() const { return _nodeType; }

        const QString &name() const { return _name; }

        void setName(const QString &name);

        bool isExtracted() const { return _isExtracted; }

        void setExtracted(bool extracted);

        bool isActive() const { return _isActive; }

        void setActive(bool active);

        bool isMovable() const override { return true; }

        bool isResizable() const override { return true; }

        bool isCopyable() const override { return true; }

        bool isDeletable() const override { return true; }

        void startSize();

        void setCorners(QPoint topLeft, QPoint bottomRight) override;

        void doSizeAction();

        virtual void attachRuntime(MaximCompiler::Runtime *runtime, MaximCompiler::Transaction *transaction) = 0;

        const std::optional<NodeCompileMeta> &compileMeta() const { return _compileMeta; }

        void setCompileMeta(std::optional<NodeCompileMeta> compileMeta) { _compileMeta = std::move(compileMeta); }

        virtual void updateRuntimePointers(MaximCompiler::Runtime *runtime, void *surfacePtr);

        void doRuntimeUpdate() override;

        void remove() override;

    private:
        NodeSurface *_surface;
        NodeType _nodeType;
        QString _name;
        bool _isExtracted = false;
        std::shared_ptr<AxiomCommon::Promise<ControlSurface *>> _controls;
        QRect sizeStartRect;
        std::optional<NodeCompileMeta> _compileMeta;
        uint32_t *_activeBitmap = nullptr;
        bool _isActive = true;
    };
}
