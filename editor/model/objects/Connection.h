#pragma once

#include "../ModelObject.h"
#include "../ConnectionWire.h"

namespace AxiomModel {

    class NodeSurface;

    class Control;

    class Connection : public ModelObject {
    public:
        Connection(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlA, const QUuid &controlB,
                   ModelRoot *root);

        static std::unique_ptr<Connection>
        create(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlA, const QUuid &controlB,
               ModelRoot *root);

        static std::unique_ptr<Connection>
        deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        bool buildOnRemove() const override { return true; }

        NodeSurface *surface() const { return _surface; }

        Control *controlA() const { return _controlA; }

        Control *controlB() const { return _controlB; }

        ConnectionWire &wire() { return _wire; }

        const ConnectionWire &wire() const { return _wire; }

        void attachRuntime();

        void detachRuntime();

        void remove() override;

    private:
        NodeSurface *_surface;
        Control *_controlA;
        Control *_controlB;
        ConnectionWire _wire;
    };

}
