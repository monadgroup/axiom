#pragma once

#include <QtCore/QObject>
#include <set>

#include "../common/ControlType.h"
#include "../common/ControlDirection.h"

namespace MaximCodegen {

    class Control;

}

namespace MaximRuntime {

    class ControlGroup;

    class SoftControl;

    class Node;

    class Control : public QObject {
    Q_OBJECT

    public:
        Control(Node *node, std::string name, MaximCommon::ControlType type);

        Node *node() const { return _node; }

        std::string name() const { return _name; }

        MaximCommon::ControlType type() const { return _type; }

        virtual MaximCommon::ControlDirection direction() const = 0;

        virtual MaximCodegen::Control *control() const = 0;

        ControlGroup *group() const { return _group; }

        void setGroup(ControlGroup *newGroup);

        void connectTo(Control *other);

        void disconnectFrom(Control *other);

        std::set<Control *> &connections() { return _connections; }

    public slots:

        void remove();

    signals:

        void removed();

        void cleanup();

    private:
        Node *_node;

        std::string _name;

        MaximCommon::ControlType _type;

        ControlGroup *_group = nullptr;

        std::set<Control *> _connections;
    };

}
