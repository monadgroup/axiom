#pragma once

#include <QtCore/QObject>

#include "Node.h"
#include "Schematic.h"

namespace MaximRuntime {

    class SoftControl;

    class GroupNode : public QObject, public Node {
        Q_OBJECT

    public:
        explicit GroupNode(Schematic *parent);

        Schematic &subsurface() { return _subsurface; }

        void compile() override;

        std::vector<std::unique_ptr<SoftControl>> &controls() { return _controls; }

        void addControl(std::unique_ptr<SoftControl> control);

    signals:

        void controlAdded(SoftControl *control);

    private slots:

        void removeControl(SoftControl *control);

    private:

        Schematic _subsurface;

        std::vector<std::unique_ptr<SoftControl>> _controls;

    };

}
