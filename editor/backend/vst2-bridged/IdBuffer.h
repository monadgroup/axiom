#pragma once

#include <QtCore/QString>

namespace AxiomBackend {

    constexpr size_t IO_SAMPLE_COUNT = 255;
    constexpr size_t IO_BUFFER_SIZE = IO_SAMPLE_COUNT * 2;

    uint32_t generateNewBufferId();

    QString getBufferStringKey(uint32_t id);

}
