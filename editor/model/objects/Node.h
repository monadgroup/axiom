#pragma once

#include "../ModelObject.h"
#include "../grid/GridItem.h"
#include "../Promise.h"

namespace AxiomModel {

    class ControlSurface;

    class Node : public GridItem, public ModelObject {
    public:
        enum class NodeType {
            CUSTOM_NODE,
            GROUP_NODE
        };

        Event<const QString &> nameChanged;
        Event<bool> extractedChanged;

        Node(NodeType nodeType, const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, const QUuid &controlsUuid, ModelRoot *root);

        static std::unique_ptr<Node> deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        Promise<ControlSurface*> &controls() { return _controls; }

        const Promise<ControlSurface*> &controls() const { return _controls; }

        NodeType nodeType() const { return _nodeType; }

        const QString &name() const { return _name; }

        void setName(const QString &name);

        bool isExtracted() { return _isExtracted; }

        bool isMovable() const override { return true; }

        bool isResizable() const override { return true; }

        bool isCopyable() const override { return true; }

        bool isDeletable() const override { return true; }

        void setCorners(QPoint topLeft, QPoint bottomRight) override;

        void remove() override;

    private:
        NodeType _nodeType;
        QString _name;
        bool _isExtracted = false;
        Promise<ControlSurface*> _controls;
    };

}
