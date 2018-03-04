#include "Surface.h"

#include "Runtime.h"
#include "Node.h"
#include "Control.h"
#include "../codegen/MaximContext.h"

using namespace MaximRuntime;

struct CalcControlGroup {
    std::set<Control*> controls;
};

Surface::Surface(Runtime *runtime, size_t depth)
    : RuntimeUnit(runtime, &_module), _module("surface", runtime->ctx()->llvm()), _depth(depth) {

}

MaximCodegen::ComposableModuleClass *Surface::compile() {
    if (!_needsGraphUpdate) return _class.get();

    // compile all children
    for (const auto &node : _nodes) {
        node->compile();
    }

    /// CONTROL GROUPING
    // calculate in-surface control groups


    // figure out which groups we own, and which groups are exposed to the parent

    // generate constructor, assign control ptrs from context & passed in parameters

    /// EXTRACTOR HANDLING
    // floodfill to find extracted nodes

    // generate control group -> control assignments based on extraction data

    /// NODE WALKING

    // calculate node execution order, using control read/write to make directed graph

    // instantiate nodes

    _class = std::make_unique<MaximCodegen::ComposableModuleClass>(runtime()->ctx(), module(), "surface");

}

void Surface::addNode(Node *node) {
    _nodes.emplace(node);
    scheduleGraphUpdate();
}

void Surface::removeNode(Node *node) {
    _nodes.erase(node);
    scheduleGraphUpdate();
}
