#pragma once

#include <QtCore/QObject>

#include <vector>
#include <set>

#include "RuntimeUnit.h"
#include "../common/ControlType.h"

namespace MaximCodegen {
    class ControlInstance;
}

namespace MaximRuntime {

    class Node;

    class ControlGroup;

    class Control : public QObject {
        Q_OBJECT

    public:
        Control(Node *node, const std::string &name, MaximCommon::ControlType type);

        Node *node() const { return _node; }

        std::string name() const { return _name; }

        MaximCommon::ControlType type() const { return _type; }

        ControlGroup *group() const { return _group; }

        MaximCodegen::ControlInstance *instance() const;

        void setGroup(ControlGroup *group);

        void connectTo(Control *other);

        void disconnectFrom(Control *other);

        virtual std::vector<Control*> internallyLinkedControls();

        bool exposed() const;

        virtual bool codeWritesTo() = 0;

        virtual bool codeReadsFrom() = 0;

        std::set<Control*> &connections() { return _connections; }

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
