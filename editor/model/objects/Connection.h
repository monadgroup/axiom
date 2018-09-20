#pragma once

#include "../ConnectionWire.h"
#include "../ModelObject.h"
#include "common/Promise.h"

namespace AxiomModel {

    class NodeSurface;

    class Control;

    class Connection : public ModelObject {
    public:
        Connection(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlAUuid, const QUuid &controlBUuid,
                   ModelRoot *root);

        static std::unique_ptr<Connection> create(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlA,
                                                  const QUuid &controlB, ModelRoot *root);

        bool buildOnRemove() const override { return true; }

        NodeSurface *surface() const { return _surface; }

        const QUuid &controlAUuid() const { return _controlAUuid; }

        const QUuid &controlBUuid() const { return _controlBUuid; }

        AxiomCommon::Promise<std::unique_ptr<ConnectionWire>> wire() const { return _wire; }

        void remove() override;

    private:
        NodeSurface *_surface;
        QUuid _controlAUuid;
        QUuid _controlBUuid;
        AxiomCommon::Promise<std::unique_ptr<ConnectionWire>> _wire;
    };
}
