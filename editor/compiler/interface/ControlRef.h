#pragma once

#include <QtCore/QString>

#include "editor/model/objects/Control.h"

namespace MaximCompiler {

    enum class ControlType {
        Audio,
        Graph,
        Midi,
        Roll,
        Scope,
        AudioExtract,
        MidiExtract
    };

    class ControlRef {
    public:
        explicit ControlRef(void *handle);

        void *get() const { return handle; }

        QString getName() const;

        ControlType getType() const;

        bool getIsWritten() const;

        bool getIsRead() const;

    private:
        void *handle;
    };

    ControlType fromModelType(AxiomModel::Control::ControlType modelType);

    AxiomModel::Control::ControlType toModelType(ControlType type);

}
