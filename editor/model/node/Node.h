#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPoint>
#include <QtCore/QSize>

#include "../GridItem.h"
#include "NodeSurface.h"

namespace MaximRuntime {
    class Node;
}

namespace AxiomModel {

    class Schematic;

    class Node : public GridItem {
    Q_OBJECT

    public:
        static constexpr float minPanelHeight = 40;

        enum class Type {
            CUSTOM,
            GROUP,
            OUTPUT
        };
        Type type;

        NodeSurface surface;
        Schematic *parentSchematic;

        Node(Schematic *parent, QString name, Type type, QPoint pos, QSize size);

        QString name() const { return m_name; }

        virtual MaximRuntime::Node *runtime() = 0;

        bool isPanelOpen() const { return m_panelOpen; }

        float panelHeight() const { return m_panelHeight; }

        bool isMovable() const override { return true; }

        bool isResizable() const override { return true; }

        virtual bool isDeletable() const { return true; }

    public slots:

        void doRuntimeUpdate() override;

        void setName(const QString &name);

        void setCorners(QPoint topLeft, QPoint bottomRight) override;

        void setPanelOpen(bool panelOpen);

        void setPanelHeight(float panelHeight);

    signals:

        void nameChanged(const QString &newName);

        void panelOpenChanged(bool newPanelOpen);

        void beforePanelHeightChanged(float newPanelHeight);

        void panelHeightChanged(float newPanelHeight);

    private:
        QString m_name = "";
        bool m_panelOpen = false;
        float m_panelHeight = 50;
    };

}
