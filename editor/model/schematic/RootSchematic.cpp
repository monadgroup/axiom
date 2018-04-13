#include "RootSchematic.h"

#include "../Project.h"
#include "../node/IONode.h"
#include "compiler/runtime/RootSurface.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

RootSchematic::RootSchematic(Project *project) : Schematic(project) {
    auto inNode = std::make_unique<IONode>(this, "Input", QPoint(-3, 0), MaximCommon::ControlType::MIDI);
    inputNode = inNode.get();
    addItem(std::move(inNode));

    auto outNode = std::make_unique<IONode>(this, "Output", QPoint(3, 0), MaximCommon::ControlType::NUMBER);
    outputNode = outNode.get();
    addItem(std::move(outNode));
}

QString RootSchematic::name() {
    return "Root";
}

void RootSchematic::attachRuntime(MaximRuntime::RootSurface *surface) {
    inputNode->attachRuntime(surface->input);
    outputNode->attachRuntime(surface->output);
    Schematic::attachRuntime(surface);
}
