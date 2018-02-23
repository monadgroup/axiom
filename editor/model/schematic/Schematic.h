#pragma once

#include <memory>
#include <vector>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPointF>
#include <QtCore/QTimer>

#include "../connection/ConnectionWire.h"
#include "../GridSurface.h"
namespace MaximRuntime {

    class Schematic;

}

namespace AxiomModel {

    class ModuleNode;

    class Schematic : public GridSurface {
    Q_OBJECT

    public:
        explicit Schematic(MaximRuntime::Schematic *runtime);

        virtual QString name() = 0;

        MaximRuntime::Schematic *runtime() { return _runtime; }

        QPointF pan() const { return m_pan; }

        const std::vector<std::unique_ptr<ConnectionWire>> &wires() const { return m_wires; }

        virtual void serialize(QDataStream &stream) const;

        virtual void deserialize(QDataStream &stream);

        void addWire(std::unique_ptr<ConnectionWire> wire);

    public slots:

        void setPan(QPointF pan);

        ModuleNode *groupSelection();

        ConnectionWire *connectSinks(ConnectionSink *sinkA, ConnectionSink *sinkB);

    signals:

        void nameChanged(QString newName);

        void panChanged(QPointF newPan);

        void wireAdded(ConnectionWire *connection);

        void removed();

    private slots:

        void removeWire(ConnectionWire *wire);

    private:
        std::vector<std::unique_ptr<ConnectionWire>> m_wires;
        QPointF m_pan;
        MaximRuntime::Schematic *_runtime;
    };

}
