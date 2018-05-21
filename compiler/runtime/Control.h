#pragma once

#include <vector>
#include <set>

#include "common/Event.h"
#include "RuntimeUnit.h"

namespace MaximCodegen {
    class Control;
}

namespace MaximRuntime {

    class Node;

    class ControlGroup;

    class Control {
    public:
        AxiomCommon::Event<Control *> exposerChanged;
        AxiomCommon::Event<> removed;
        AxiomCommon::Event<> cleanup;

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

        virtual int64_t instanceId() const = 0;

        void setExposer(Control *control);

        Control *exposer() const { return _exposer; }

        std::set<Control *> &connections() { return _connections; }

        void exposerCleared(Control *exposer);

        void remove();

    protected:

        virtual void onRemove() {}

    private:

        Node *_node;

        ControlGroup *_group = nullptr;

        std::set<Control *> _connections;

        Control *_exposer = nullptr;
    };

}
