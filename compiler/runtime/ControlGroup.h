#pragma once

#include <set>

#include "RuntimeUnit.h"
#include "../common/ControlType.h"

namespace MaximRuntime {

    class Surface;

    class Control;

    class ControlGroup : public RuntimeUnit {
    public:
        ControlGroup(Surface *surface, MaximCommon::ControlType type);

        MaximCommon::ControlType type() const { return _type; }

        Surface *surface() const { return _surface; }

        std::set<Control*> &controls() { return _controls; }

        void absorb(ControlGroup *other);

        void addControl(Control *control);

        void removeControl(Control *control);

    private:

        MaximCommon::ControlType _type;
        Surface *_surface;
        std::set<Control*> _controls;
    };

}
