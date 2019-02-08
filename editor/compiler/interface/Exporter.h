#pragma once

#include <QtCore/QString>
#include <optional>

#include "Frontend.h"
#include "OwnedObject.h"
#include "Transaction.h"

namespace MaximCompiler {

    class AudioConfig : public OwnedObject {
    public:
        AudioConfig(double sampleRate, double bpm);
    };

    class TargetConfig : public OwnedObject {
    public:
        TargetConfig(MaximFrontend::TargetPlatform platform, MaximFrontend::TargetInstructionSet instructionSet,
                     MaximFrontend::FeatureLevel featureLevel);
    };

    class CodeConfig : public OwnedObject {
    public:
        CodeConfig(MaximFrontend::OptimizationLevel optimizationLevel, const QString &instrumentPrefix,
                   bool includeInstrument, bool includeLibrary);
    };

    class ObjectOutputConfig : public OwnedObject {
    public:
        explicit ObjectOutputConfig(MaximFrontend::ObjectFormat format, const QString &location);
    };

    class MetaOutputConfig : public OwnedObject {
    public:
        MetaOutputConfig(MaximFrontend::MetaFormat format, const QString &location, QString *portalNames,
                         size_t portalNameCount);
    };

    class ExportConfig : public OwnedObject {
    public:
        ExportConfig(AudioConfig audio, TargetConfig target, CodeConfig code,
                     std::optional<ObjectOutputConfig> objectOutput, std::optional<MetaOutputConfig> metaOutput);
    };

    class Exporter {
    public:
        static bool exportTransaction(const ExportConfig &config, Transaction transaction);
    };
}
