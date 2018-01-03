#pragma once

#include "GridItem.h"

namespace AxiomModel {

    class Control : public GridItem {
    Q_OBJECT

    public:
        enum class Type {
            Plug,
            Label,
            Knob,
            Slider,
            CurveEditor
        };

        const Type type;

        explicit Control(GridSurface *parent, Type type);
    };

}
