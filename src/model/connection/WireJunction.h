#pragma once

#include <QtCore/QObject>

#include "ConnectionSource.h"
#include "ConnectionSink.h"

namespace AxiomModel {

    class WireJunction : public QObject {
    Q_OBJECT

    public:
        const ConnectionSource source;
        const ConnectionSink sink;
    };

}
