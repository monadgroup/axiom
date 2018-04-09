#pragma once

#include <memory>
#include <vector>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPointF>
#include <QtCore/QTimer>

#include "../connection/ConnectionWire.h"
#include "../GridSurface.h"
#include "../Ref.h"
#include "../node/Node.h"

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
        explicit Schematic(Project *project, SurfaceRef ref, MaximRuntime::Surface *runtime);

        virtual QString name() = 0;

        const SurfaceRef &ref() const { return _ref; }

        MaximRuntime::Surface *runtime() { return _runtime; }

        QPointF pan() const { return m_pan; }

        float zoom() const { return m_zoom; }

        Project *project() const { return _project; }

        const std::vector<std::unique_ptr<ConnectionWire>> &wires() const { return m_wires; }

        void addWire(std::unique_ptr<ConnectionWire> wire);

        virtual bool canExposeControl() = 0;

        virtual void exposeControl(AxiomModel::NodeControl *control) = 0;

        Node *addFromStream(Node::Type type, QDataStream &stream);

    public slots:

        void setPan(QPointF pan);

        void setZoom(float zoom);

        GroupNode *groupSelection();

        ConnectionWire *connectSinks(ConnectionSink *sinkA, ConnectionSink *sinkB);

        virtual void serialize(QDataStream &stream) const;

        virtual void deserialize(QDataStream &stream);

        void addNode(Node::Type type, QString name, QPoint pos);

    signals:

        void nameChanged(QString newName);

        void panChanged(QPointF newPan);

        void zoomChanged(float newZoom);

        void wireAdded(ConnectionWire *connection);

        void removed();

        void cleanup();

    protected slots:

        void remove();

    private slots:

        void removeWire(ConnectionWire *wire);

    private:
        Project *_project;
        SurfaceRef _ref;
        std::vector<std::unique_ptr<ConnectionWire>> m_wires;
        QPointF m_pan;
        float m_zoom = 0;
        MaximRuntime::Surface *_runtime;
    };

}
