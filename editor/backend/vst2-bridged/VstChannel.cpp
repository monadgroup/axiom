#include "VstChannel.h"

using namespace AxiomBackend;

VstChannel::SeparateData::SeparateData(const std::string &id)
    : guiVstToAppData(id + ".guiVstToApp"),
      guiAppToVstData(id + ".guiAppToVst"),
      audioVstToAppData(id + ".audioVstToApp"),
      audioAppToVstData(id + ".audioAppToVst") {}
