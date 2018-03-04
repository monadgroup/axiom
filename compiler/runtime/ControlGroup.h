#pragma once

#include <set>

#include "RuntimeUnit.h"

namespace MaximCodegen {
    class Control;
}

namespace MaximRuntime {

    class Surface;

    class Control;

    class ControlGroup : public RuntimeUnit {
    public:
        ControlGroup(Surface *surface, MaximCodegen::Control *type);

        MaximCodegen::Control *type() const { return _type; }

        Surface *surface() const { return _surface; }

        std::set<Control*> &controls() { return _controls; }

        void absorb(ControlGroup *other);

        void addControl(Control *control);

        void removeControl(Control *control);

        bool exposed() const;

        bool writtenTo() const;

        bool readFrom() const;

        bool extracted() const;

        void setExtracted(bool extracted);

    private:

        MaximCodegen::Control *_type;
        Surface *_surface;
        std::set<Control*> _controls;
    };

}
