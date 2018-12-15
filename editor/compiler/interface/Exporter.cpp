#include "Exporter.h"

#include "Frontend.h"

using namespace MaximCompiler;

void Exporter::exportTransaction(bool minSize, MaximCompiler::Transaction transaction) {
    MaximFrontend::maxim_export_transaction(minSize, transaction.release());
}
