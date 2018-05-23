#pragma once

#include "../ModelObject.h"
#include "../ConnectionWire.h"
#include "common/Promise.h"

namespace AxiomModel {

    class NodeSurface;

    class Control;

    class Connection : public ModelObject {
    public:
        Connection(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlAUuid, const QUuid &controlBUuid,
                   ModelRoot *root);

        static std::unique_ptr<Connection>
        create(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlA, const QUuid &controlB,
               ModelRoot *root);

        static std::unique_ptr<Connection>
        deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, ReferenceMapper *ref, ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        bool buildOnRemove() const override { return true; }

        NodeSurface *surface() const { return _surface; }

        const QUuid &controlAUuid() const { return _controlAUuid; }

        const QUuid &controlBUuid() const { return _controlBUuid; }

        AxiomCommon::Promise<ConnectionWire> wire() const { return _wire; }

        void attachRuntime();

        void detachRuntime();

        void remove() override;

    private:
        NodeSurface *_surface;
        QUuid _controlAUuid;
        QUuid _controlBUuid;
        AxiomCommon::Promise<ConnectionWire> _wire;
    };

}
