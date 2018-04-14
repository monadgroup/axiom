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
        explicit Schematic(Project *project);

        virtual QString name() = 0;

        virtual SurfaceRef ref() const = 0;

        MaximRuntime::Surface *runtime() const { return _runtime; }

        void attachRuntime(MaximRuntime::Surface *runtime);

        QPointF pan() const { return m_pan; }

        float zoom() const { return m_zoom; }

        Project *project() const { return _project; }

        const std::vector<std::unique_ptr<ConnectionWire>> &wires() const { return m_wires; }

        void addWire(std::unique_ptr<ConnectionWire> wire);

        virtual bool canExposeControl() = 0;

        virtual void exposeControl(AxiomModel::NodeControl *control) = 0;

        Node *addFromStream(Node::Type type, size_t index, QDataStream &stream, QPoint center);

        static void partialSerialize(QDataStream &stream, const std::vector<GridItem*> &items, QPoint center);

    public slots:

        void setPan(QPointF pan);

        void setZoom(float zoom);

        void partialDeserialize(QDataStream &stream, QPoint center);

        void copyIntoSelf(const std::vector<GridItem*> &items, QPoint center);

        void connectControls(NodeControl *controlA, NodeControl *controlB);

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
        std::vector<std::unique_ptr<ConnectionWire>> m_wires;
        QPointF m_pan;
        float m_zoom = 0;
        MaximRuntime::Surface *_runtime = nullptr;
    };

}
