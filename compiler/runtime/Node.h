#pragma once

#include <set>
#include <QtCore/QObject>

#include "ModuleRuntimeUnit.h"

namespace MaximRuntime {

    class GeneratableModuleClass;

    class Surface;

    class Control;

    class Node : public QObject, public ModuleRuntimeUnit {
        Q_OBJECT

    public:
        explicit Node(Surface *surface);

        virtual GeneratableModuleClass *compile() = 0;

        virtual void remove();

        Surface *surface() const { return _surface; }

        virtual const std::unique_ptr<Control> *begin() const = 0;

        virtual const std::unique_ptr<Control> *end() const = 0;

        void scheduleCompile();

        bool needsCompile() const { return _needsCompile; }

        bool extracted() const { return _extracted; }

    public slots:

        void setExtracted(bool extracted);

    signals:

        void extractedChanged(bool newExtracted);

    protected:

        bool _needsCompile = false;

    private:

        Surface *_surface;

        bool _extracted = false;
    };

}
