#pragma once

#include "../ModelObject.h"
#include "../grid/GridItem.h"
#include "../CollectionView.h"

namespace AxiomModel {

    class ControlSurface;

    class Connection;

    class Control : public GridItem, public ModelObject {
    public:
        enum class ControlType {
            NUM_SCALAR,
            MIDI_SCALAR
        };

        Event<const QString &> nameChanged;
        Event<QPointF> worldPosChanged;

        Control(ControlType controlType, ConnectionWire::WireType wireType, QUuid uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name, ModelRoot *root);

        static std::unique_ptr<Control> deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        ControlSurface *surface() const { return _surface; }

        ControlType controlType() const { return _controlType; }

        ConnectionWire::WireType wireType() const { return _wireType; }

        bool isMovable() const override { return true; }

        bool isResizable() const override { return true; }

        bool isCopyable() const override { return false; }

        bool isDeletable() const override { return false; }

        const QString &name() const { return _name; }

        void setName(const QString &name);

        CollectionView<Connection *> &connections() { return _connections; }

        const CollectionView<Connection *> &connections() const { return _connections; }

        CollectionView<Control *> &connectedControls() { return _connectedControls; }

        const CollectionView<Control *> &connectedControls() const { return _connectedControls; }

        QPointF worldPos() const;

        void remove() override;

    private:
        ControlSurface *_surface;
        ControlType _controlType;
        ConnectionWire::WireType _wireType;
        QString _name;

        CollectionView<Connection *> _connections;
        CollectionView<Control *> _connectedControls;

        void updateSinkPos();
    };

}
