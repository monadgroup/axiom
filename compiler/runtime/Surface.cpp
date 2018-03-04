#include "Surface.h"

#include <unordered_set>
#include <queue>

#include "Runtime.h"
#include "Node.h"
#include "Control.h"
#include "ControlGroup.h"
#include "../codegen/MaximContext.h"
#include "../codegen/Control.h"
#include "../codegen/Scope.h"

using namespace MaximRuntime;

Surface::Surface(Runtime *runtime, size_t depth)
    : ModuleRuntimeUnit(runtime, "surface"), _depth(depth) {

}

void Surface::scheduleGraphUpdate() {
    _needsGraphUpdate = true;
    scheduleCompile();
}

void Surface::addNode(Node *node) {
    _nodes.emplace(node);
    scheduleGraphUpdate();
}

void Surface::removeNode(Node *node) {
    _nodes.erase(node);
    scheduleGraphUpdate();
}

void Surface::addControlGroup(std::unique_ptr<ControlGroup> group) {
    _controlGroups.push_back(std::move(group));
    scheduleGraphUpdate();
}

std::unique_ptr<ControlGroup> Surface::removeControlGroup(ControlGroup *group) {
    scheduleGraphUpdate();

    for (auto i = _controlGroups.begin(); i < _controlGroups.end(); i++) {
        if (i->get() == group) {
            auto movedGroup = std::move(*i);
            _controlGroups.erase(i);
            return movedGroup;
        }
    }

    assert(false);
    throw;
}

MaximCodegen::ModuleClass* Surface::doCompile() {
    if (!_needsGraphUpdate) return _class.get();

    reset();

    // compile all children
    std::unordered_map<Node*, MaximCodegen::ComposableModuleClass *> nodeClasses;
    for (const auto &node : _nodes) {
        nodeClasses.emplace(node, node->compile());
    }

    /// CONTROL GROUPING
    // figure out which groups we own, and which groups are exposed to the parent
    std::vector<ControlGroup *> ownedGroups;
    std::vector<ControlGroup *> exposedGroups;
    std::vector<llvm::Type *> exposedParamTypes;
    for (const auto &group : _controlGroups) {
        if (group->exposed()) {
            exposedGroups.push_back(group.get());
            exposedParamTypes.push_back(group->type()->storageType());
        } else {
            ownedGroups.push_back(group.get());
        }
    }

    /// EXTRACTOR HANDLING
    // floodfill to find extracted nodes
    std::unordered_set<Node*> extractedNodes;
    std::unordered_set<Control*> visitedControls;
    std::queue<Control*> controlQueue;

    // step 1: find extractor controls
    for (const auto &node : _nodes) {
        for (const auto &control : node->controls()) {
            if ((uint32_t) control->type() & (uint32_t) MaximCommon::ControlType::EXTRACT) {
                controlQueue.emplace(control);
                visitedControls.emplace(control);
            }
        }
    }

    // step 2: breadth-first search from writes -> reads to find any controls that should be extractors
    while (!controlQueue.empty()) {
        auto nextControl = controlQueue.front();
        controlQueue.pop();

        // find any connections that read and aren't extractors
        for (const auto &connectedControl : nextControl->connections()) {
            if (!connectedControl->codeReadsFrom()) continue;
            if ((uint32_t) connectedControl->type() & (uint32_t) MaximCommon::ControlType::EXTRACT) continue;
            if (visitedControls.find(connectedControl) != visitedControls.end()) continue;

            extractedNodes.emplace(connectedControl->node());
            visitedControls.emplace(connectedControl);
            controlQueue.emplace(connectedControl);
        }
    }

    // step 3: mark control groups as extracted if necessary
    for (const auto &group : _controlGroups) {
        // a group is only extracted if _all_ writers are extracted
        bool isExtracted = true;
        for (const auto &control : group->controls()) {
            if (!control->codeWritesTo()) continue;
            if (extractedNodes.find(control->node()) == extractedNodes.end()) {
                isExtracted = false;
                break;
            }
        }

        group->setExtracted(isExtracted);
    }

    // generate constructor, assign control ptrs from context & passed in parameters
    _class = std::make_unique<MaximCodegen::ComposableModuleClass>(runtime()->ctx(), module(), "surface", exposedParamTypes);
    //MaximCodegen::ComposableModuleClassMethod generateMethod(_class.get(), "generate");

    std::unordered_map<ControlGroup *, llvm::Value *> groupConstructorPtrs;

    for (const auto &ownedGroup : ownedGroups) {
        auto groupIndex = _class->addEntry(ownedGroup->compile());
        groupConstructorPtrs.emplace(ownedGroup, _class->cconstructor()->getEntryPointer(groupIndex, "group"));
    }
    for (size_t i = 0; i < exposedGroups.size(); i++) {
        groupConstructorPtrs.emplace(exposedGroups[i], _class->cconstructor()->arg(i));
    }

    /// NODE WALKING
    // calculate node execution order, using control read/write to make directed graph
    std::vector<Node *> inverseExecutionOrder;
    std::unordered_set<Node *> visitedNodes;
    std::queue<Node *> nodeQueue;

    // step 1: find exposed controls in groups that are written to
    for (const auto &group : _controlGroups) {
        if (!group->writtenTo() || !group->exposed()) continue;

        for (const auto &control : group->controls()) {
            if (!control->exposed() || visitedNodes.find(control->node()) != visitedNodes.end()) continue;

            visitedNodes.emplace(control->node());
            nodeQueue.emplace(control->node());
        }
    }

    // breadth-first search any connected nodes
    while (!nodeQueue.empty()) {
        auto nextNode = nodeQueue.front();
        nodeQueue.pop();

        inverseExecutionOrder.push_back(nextNode);

        for (const auto &control : nextNode->controls()) {
            for (const auto &connectedControl : control->connections()) {
                // only propagate to the node if it writes to the group - other nodes won't affect output
                // (although they'll still be included in the final list by the check after this one)
                if (!connectedControl->codeWritesTo() ||
                    visitedNodes.find(connectedControl->node()) != visitedNodes.end()) continue;

                visitedNodes.emplace(connectedControl->node());
                nodeQueue.emplace(connectedControl->node());
            }
        }
    }

    // for UX, add nodes that don't affect the output (ie aren't connected in the flow)
    // so the user can still play with them - we can't really determine the order of these,
    // since there's no output. technically we could follow the directions of connections,
    // but ultimately that's probably futile
    if (inverseExecutionOrder.size() < _nodes.size()) {
        for (const auto &node : _nodes) {
            if (visitedNodes.find(node) == visitedNodes.end()) {
                inverseExecutionOrder.push_back(node);
            }
        }
    }

    // instantiate nodes in their orders - extracted nodes are created multiple times
    // control values are also assigned
    for (size_t i = inverseExecutionOrder.size() - 1; i >= 0; i--) {
        auto node = inverseExecutionOrder[i];
        auto nodeClass = nodeClasses.find(node)->second;

        // instantiate the node n times
        // todo: this loop should probably be in IR, add array entry to the class
        auto isExtracted = extractedNodes.find(node) != extractedNodes.end();
        auto loopSize = isExtracted ? MaximCodegen::ArrayType::arraySize : 1;

        for (unsigned int instN = 0; instN < loopSize; instN++) {
            auto entryIndex = _class->addEntry(nodeClass); // todo: GroupNodes will need ControlGroups passed into their constructor
            // todo: callInto node generate function
            auto entryPtr = _class->cconstructor()->getEntryPointer(entryIndex, "nodeinst");

            // setup control values in constructor
            for (const auto &control : node->controls()) {
                auto controlIndex = control->instance()->instId;
                auto controlPtr = nodeClass->getEntryPointer(_class->constructor()->builder(), controlIndex, entryPtr, "controlinst");

                auto groupPtr = groupConstructorPtrs.find(control->group())->second;
                auto indexPtr = control->group()->extracted()
                                ? _class->constructor()->builder().CreateConstGEP2_32(control->group()->type()->storageType(), groupPtr, 0, instN)
                                : groupPtr;

                _class->constructor()->builder().CreateStore(indexPtr, controlPtr);
            }
        }
    }

    deploy();
    return _class.get();
}
