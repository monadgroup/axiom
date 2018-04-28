#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPoint>
#include <QtCore/QSize>

#include "../GridItem.h"
#include "NodeSurface.h"
#include "../Ref.h"

namespace MaximRuntime {
    class Node;
    class Surface;
    class Control;
}

namespace AxiomModel {

    class Schematic;

    class NodeControl;

    class Node : public GridItem {
    Q_OBJECT

    public:
        static constexpr float minPanelHeight = 40;

        enum class Type {
            CUSTOM,
            GROUP,
            IO
        };
        Type type;

        NodeSurface surface;
        Schematic *parentSchematic;

        Node(Schematic *parent, QString name, Type type, QPoint pos, QSize size);

        NodeRef ref() const;

        QString name() const { return m_name; }

        virtual MaximRuntime::Node *runtime() = 0;

        virtual void createAndAttachRuntime(MaximRuntime::Surface *surface) = 0;

        bool isExtracted();

        bool isPanelOpen() const { return m_panelOpen; }

        float panelHeight() const { return m_panelHeight; }

        bool isMovable() const override { return true; }

        bool isResizable() const override { return true; }

        bool isCopyable() const override { return true; }

        bool isDeletable() const override { return true; }

    public slots:

        void doRuntimeUpdate() override;

        void saveValue() override;

        void restoreValue() override;

        void setName(const QString &name);

        void setNameNoOp(const QString &name);

        void setCorners(QPoint topLeft, QPoint bottomRight) override;

        void startResize();

        void finishResize();

        void startDragging() override;

        void finishDragging() override;

        void setPanelOpen(bool panelOpen);

        void setPanelHeight(float panelHeight);

        void serialize(QDataStream &stream, QPoint offset) const override;

        void deserialize(QDataStream &stream, QPoint offset) override;

        void remove() override;

        void removeWithoutOp();

    signals:

        void nameChanged(const QString &newName);

        void extractedChanged(bool newExtracted);

        void panelOpenChanged(bool newPanelOpen);

        void beforePanelHeightChanged(float newPanelHeight);

        void panelHeightChanged(float newPanelHeight);

    protected:

        NodeControl *addFromRuntime(MaximRuntime::Control *control);

    private:
        QString m_name = "";
        bool m_panelOpen = false;
        float m_panelHeight = 50;

        QPoint startDragPos;
        QPoint startResizeTopLeft;
        QPoint startResizeBottomRight;
        std::vector<QPoint> controlStartPos;
    };

}
