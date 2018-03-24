#pragma once

#include <QtCore/QObject>

#include <vector>
#include <set>

#include "RuntimeUnit.h"

namespace MaximCodegen {
    class Control;
}

namespace MaximRuntime {

    class Node;

    class ControlGroup;

    class Control : public QObject {
    Q_OBJECT

    public:
        explicit Control(Node *node);

        Node *node() const { return _node; }

        virtual std::string name() const = 0;

        virtual MaximCodegen::Control *type() const = 0;

        virtual bool writtenTo() const = 0;

        virtual bool readFrom() const = 0;

        ControlGroup *group() const { return _group; }

        void setGroup(ControlGroup *group);

        void connectTo(Control *other);

        void disconnectFrom(Control *other);

        virtual void addInternallyLinkedControls(std::set<Control *> &controls) = 0;

        Control *exposer() const;

        std::set<Control *> &connections() { return _connections; }

    public slots:

        void remove();

    signals:

        void removed();

        void cleanup();

    private:

        Node *_node;

        ControlGroup *_group = nullptr;

        std::set<Control *> _connections;
    };

}
