#pragma once

#include "../ModelObject.h"
#include "../Promise.h"
#include "../ConnectionWire.h"

namespace AxiomModel {

    class NodeSurface;

    class Control;

    class Connection : public ModelObject {
    public:
        Connection(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlA, const QUuid &controlB, ModelRoot *root);

        static std::unique_ptr<Connection> deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        NodeSurface *surface() const { return _surface; }

        const QUuid &controlAUuid() const { return _controlAUuid; }

        Promise<Control*> &controlA() { return _controlA; }

        const Promise<Control*> &controlA() const { return _controlA; }

        const QUuid &controlBUuid() const { return _controlBUuid; }

        Promise<Control*> &controlB() { return _controlB; }

        const Promise<Control*> &controlB() const { return _controlB; }

        const ConnectionWire &wire() const { return _wire; }

    private:
        NodeSurface *_surface;
        QUuid _controlAUuid;
        Promise<Control*> _controlA;
        QUuid _controlBUuid;
        Promise<Control*> _controlB;
        ConnectionWire _wire;
    };

}
