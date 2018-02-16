#pragma once

#include <vector>

#include "../common/ControlType.h"

namespace llvm {
    class GlobalVariable;
}

namespace MaximRuntime {

    class Surface;

    class Control;

    class ControlGroup {
    public:

        ControlGroup(Surface *surface, MaximCommon::ControlType type, Control *writer, std::vector<Control*> controls, bool isExposed);

        ~ControlGroup();

        explicit ControlGroup(Control *control);

        void findWriter(Control *ignore = nullptr);

        void updateExposed();

        Surface *surface() const { return _surface; }

        void setSurface(Surface *surface) { _surface = surface; }

        Control *writer() const { return _writer; }

        void setWriter(Control *writer) { _writer = writer; }

        std::vector<Control*> &controls() { return _controls; }

        bool isExposed() const { return _isExposed; }

        void setExposed(bool exposed) { _isExposed = exposed; }

        llvm::GlobalVariable *global() const { return _global; }

    private:
        static size_t _nextId;

        Surface *_surface;
        MaximCommon::ControlType _type;
        Control *_writer;
        std::vector<Control*> _controls;
        llvm::GlobalVariable *_global;
        bool _isExposed;

        size_t _id;

        void setupGlobal();
    };

}
