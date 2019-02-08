#include "Exporter.h"

#include "Frontend.h"

using namespace MaximCompiler;

AudioConfig::AudioConfig(double sampleRate, double bpm)
    : OwnedObject(MaximFrontend::maxim_create_audio_config(sampleRate, bpm),
                  &MaximFrontend::maxim_destroy_audio_config) {}

TargetConfig::TargetConfig(MaximFrontend::TargetPlatform platform, MaximFrontend::TargetInstructionSet instructionSet,
                           MaximFrontend::FeatureLevel featureLevel)
    : OwnedObject(MaximFrontend::maxim_create_target_config(platform, instructionSet, featureLevel),
                  &MaximFrontend::maxim_destroy_target_config) {}

CodeConfig::CodeConfig(MaximFrontend::OptimizationLevel optimizationLevel, const QString &instrumentPrefix,
                       bool includeInstrument, bool includeLibrary)
    : OwnedObject(MaximFrontend::maxim_create_code_config(optimizationLevel, instrumentPrefix.toUtf8().constData(),
                                                          includeInstrument, includeLibrary),
                  &MaximFrontend::maxim_destroy_code_config) {}

ObjectOutputConfig::ObjectOutputConfig(MaximFrontend::ObjectFormat format, const QString &location)
    : OwnedObject(MaximFrontend::maxim_create_object_output_config(format, location.toUtf8().constData()),
                  &MaximFrontend::maxim_destroy_object_output_config) {}

static void *createMetaOutputConfig(MaximFrontend::MetaFormat format, const QString &location, QString *portalNames,
                                    size_t portalNameCount) {
    std::vector<QByteArray> portalNameArrays;
    std::vector<const char *> portalNamePtrs;
    portalNameArrays.reserve(portalNameCount);
    portalNamePtrs.reserve(portalNameCount);
    for (size_t i = 0; i < portalNameCount; i++) {
        auto byteArray = portalNames[i].toUtf8();
        portalNamePtrs.push_back(byteArray.constData());
        portalNameArrays.push_back(std::move(byteArray));
    }

    return MaximFrontend::maxim_create_meta_output_config(format, location.toUtf8().constData(), &portalNamePtrs[0],
                                                          portalNameCount);
}

MetaOutputConfig::MetaOutputConfig(MaximFrontend::MetaFormat format, const QString &location, QString *portalNames,
                                   size_t portalNameCount)
    : OwnedObject(createMetaOutputConfig(format, location, portalNames, portalNameCount),
                  &MaximFrontend::maxim_destroy_meta_output_config) {}

template<class T>
static void *releaseOrNull(std::optional<T> val) {
    if (val) {
        return val->release();
    } else {
        return nullptr;
    }
}

ExportConfig::ExportConfig(MaximCompiler::AudioConfig audio, MaximCompiler::TargetConfig target,
                           MaximCompiler::CodeConfig code,
                           std::optional<MaximCompiler::ObjectOutputConfig> objectOutput,
                           std::optional<MaximCompiler::MetaOutputConfig> metaOutput)
    : OwnedObject(MaximFrontend::maxim_create_export_config(audio.release(), target.release(), code.release(),
                                                            releaseOrNull(std::move(objectOutput)),
                                                            releaseOrNull(std::move(metaOutput))),
                  &MaximFrontend::maxim_destroy_export_config) {}

bool Exporter::exportTransaction(const ExportConfig &config, MaximCompiler::Transaction transaction) {
    return MaximFrontend::maxim_export_transaction(config.get(), transaction.release());
}
