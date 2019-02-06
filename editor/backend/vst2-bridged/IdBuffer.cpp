#include "IdBuffer.h"

#include <QtCore/QRandomGenerator>

uint32_t AxiomBackend::generateNewBufferId() {
    return QRandomGenerator::global()->bounded(0u, UINT32_MAX);
}

QString AxiomBackend::getBufferStringKey(uint32_t id) {
    return "monad.axiom.vst2_shared." + QString::number(id, 16).rightJustified(8, '0');
}
