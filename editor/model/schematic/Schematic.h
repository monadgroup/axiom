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

    class Surface;

}

namespace AxiomModel {

    class GroupNode;

    class NodeControl;

    class Project;

    class Schematic : public GridSurface {
    Q_OBJECT

    public:
        explicit Schematic(Project *project, MaximRuntime::Surface *runtime);

        virtual QString name() = 0;

        MaximRuntime::Surface *runtime() { return _runtime; }

        QPointF pan() const { return m_pan; }

        Project *project() const { return _project; }

        const std::vector<std::unique_ptr<ConnectionWire>> &wires() const { return m_wires; }

        void addWire(std::unique_ptr<ConnectionWire> wire);

        virtual bool canExposeControl() = 0;

        virtual void exposeControl(AxiomModel::NodeControl *control) = 0;

    public slots:

        void setPan(QPointF pan);

        GroupNode *groupSelection();

        ConnectionWire *connectSinks(ConnectionSink *sinkA, ConnectionSink *sinkB);

        virtual void serialize(QDataStream &stream) const;

        virtual void deserialize(QDataStream &stream);

    signals:

        void nameChanged(QString newName);

        void panChanged(QPointF newPan);

        void wireAdded(ConnectionWire *connection);

        void removed();

        void cleanup();

    protected slots:

        void remove();

    private slots:

        void removeWire(ConnectionWire *wire);

    private:
        Project *_project;
        std::vector<std::unique_ptr<ConnectionWire>> m_wires;
        QPointF m_pan;
        MaximRuntime::Surface *_runtime;
    };

}
