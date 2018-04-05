#include "Surface.h"

#include <unordered_set>
#include <queue>
#include <iostream>

#include "Runtime.h"
#include "HardControl.h"
#include "ControlGroup.h"
#include "GeneratableModuleClass.h"
#include "../codegen/Control.h"

using namespace MaximRuntime;

Surface::Surface(Runtime *runtime, size_t depth)
    : ModuleRuntimeUnit(runtime, "surface"), _depth(depth) {

}

void Surface::scheduleGraphUpdate() {
    _needsGraphUpdate = true;
}

bool Surface::needsGraphUpdate() const {
    if (_needsGraphUpdate || !_class) {
        std::cout << "Compiling surface because needsGraphUpdate or no class" << std::endl;
        return true;
    }
    for (const auto &node : _nodes) {
        if (node->needsCompile()) {
            std::cout << "Compiling surface because a node needs it" << std::endl;
            return true;
        }
    }
    return false;
}

GeneratableModuleClass *Surface::compile() {
    if (!needsGraphUpdate()) {
        std::cout << "Skipping surface compile, nothing's changed" << std::endl;
        return _class.get();
    }
    _needsGraphUpdate = false;

    reset();

    // compile all children
    auto nodeCodegenStart = std::clock();
    std::unordered_map<Node *, GeneratableModuleClass *> nodeClasses;
    for (const auto &node : _nodes) {
        nodeClasses.emplace(node, node->compile());
        node->setExtracted(false);
    }
    auto nodeCodegenTime = std::clock() - nodeCodegenStart;
    std::cerr << "    Finished node codegen in " << nodeCodegenTime << " ms" << std::endl;

    /// CONTROL GROUPING
    // walk the graph to find control groups
    auto controlGroupingStart = std::clock();

    // walk through new groups
    std::vector<std::unique_ptr<ControlGroup>> newGroups;
    for (const auto &node : _nodes) {
        for (const std::unique_ptr<Control> &control : *node) {
            auto newGroup = std::make_unique<ControlGroup>(this, control->type());
            auto newGroupPtr = newGroup.get();
            newGroups.emplace_back(std::move(newGroup));
            control->setGroup(newGroupPtr);
        }
    }

    // remove any empty groups
    for (auto i = newGroups.begin(); i < newGroups.end(); i++) {
        auto &group = *i;
        if (group->controls().empty()) {
            newGroups.erase(i);
            i--;
        }
    }

    _controlGroups = std::move(newGroups);

    for (const auto &node : _nodes) {
        for (const std::unique_ptr<Control> &control : *node) {

            // first absorb groups from real connections
            for (const auto &connectedControl : control->connections()) {
                control->group()->absorb(connectedControl->group());
            }

            // next absorb groups from internal connections
            std::set<Control *> internalConnections;
            control->addInternallyLinkedControls(internalConnections);
            for (const auto &connectedControl : internalConnections) {
                control->group()->absorb(connectedControl->group());
            }
        }
    }
    auto controlGroupingTime = std::clock() - controlGroupingStart;
    std::cerr << "    Finished control grouping in " << controlGroupingTime << " ms" << std::endl;

    /// EXTRACTOR HANDLING
    // floodfill to find extracted nodes
    auto extractorSearchStart = std::clock();
    std::unordered_set<Control *> visitedControls;
    std::queue<Control *> controlQueue;

    // step 1: find extractor controls
    for (const auto &node : _nodes) {
        for (const std::unique_ptr<Control> &control : *node) {
            if ((uint32_t) control->type()->type() & (uint32_t) MaximCommon::ControlType::EXTRACT) {
                controlQueue.emplace(control.get());
                visitedControls.emplace(control.get());
            }
        }
    }

    // step 2: breadth-first search from writes -> reads to find any controls that should be extractors
    while (!controlQueue.empty()) {
        auto nextControl = controlQueue.front();
        controlQueue.pop();

        for (const std::unique_ptr<Control> &relatedControl : *nextControl->node()) {
            if (visitedControls.find(relatedControl.get()) != visitedControls.end()) continue;
            visitedControls.emplace(relatedControl.get());

            if ((uint32_t) relatedControl->type()->type() & (uint32_t) MaximCommon::ControlType::EXTRACT) continue;

            controlQueue.emplace(relatedControl.get());
        }

        // find any connections that read and aren't extractors
        // if a connection doesn't read but is connected straight to an extractor, we also need to set it as extracted
        for (const auto &connectedControl : nextControl->connections()) {
            if (visitedControls.find(connectedControl) != visitedControls.end()) continue;
            visitedControls.emplace(connectedControl);

            if (!connectedControl->readFrom()) {
                // if the control writes to a group that includes an extractor, it has to be an extractor
                bool groupHasExtractor = false;
                for (const auto &groupControl : connectedControl->group()->controls()) {
                    if ((uint32_t) groupControl->type()->type() & (uint32_t) MaximCommon::ControlType::EXTRACT) {
                        groupHasExtractor = true;
                        break;
                    }
                }

                if (!groupHasExtractor) continue;
            }
            if ((uint32_t) connectedControl->type()->type() & (uint32_t) MaximCommon::ControlType::EXTRACT) continue;

            connectedControl->node()->setExtracted(true);
            controlQueue.emplace(connectedControl);
        }
    }

    // step 3: mark control groups as extracted if necessary
    for (const auto &group : _controlGroups) {
        // a group is only extracted if _all_ writers are extracted or the group has an extractor control
        // if there are no writers, the group is not extracted
        bool isExtracted = group->writtenTo();
        for (const auto &control : group->controls()) {
            if ((uint32_t) control->type()->type() & (uint32_t) MaximCommon::ControlType::EXTRACT) {
                isExtracted = true;
                break;
            }
            if (!control->writtenTo()) continue;
            if (!control->node()->extracted()) {
                isExtracted = false;
                break;
            }
        }

        group->setExtracted(isExtracted);
    }

    auto extractorSearchTime = std::clock() - extractorSearchStart;
    std::cerr << "    Finished extractor search in " << extractorSearchTime << " ms" << std::endl;

    /// CONTROLGROUP INITIALIZATION
    auto groupInitializationStart = std::clock();
    _class = std::make_unique<GeneratableModuleClass>(runtime()->ctx(), module(), "surface");
    _groupPtrIndexes.clear();
    for (const auto &group : _controlGroups) {
        // There are two types of groups: those that we own, and those that are exposed (ie are 'owned' by the parent,
        // we don't instantiate them here).
        // In the end, we want to have an entry that's a pointer to the group value.
        // For OWNED groups, we also need to actually store the value. So we create two entries, one for the value and
        // the other for the pointer. We also add some code to the constructor to set the pointer entry.
        // For EXPOSED groups, we just add an entry for a pointer to the value. We also record the group and entry index
        // in the _groupPtrIndexes map, which is used by the parent GroupNode (if there is one) to assign instance
        // indexes that are then read by whatever surface is above us, and used to assign our pointers to the correct
        // values.
        //
        // It's complicated, I know.

        size_t entryIndex;

        if (group->exposed()) {
            // just add an entry for a pointer to the group
            // todo: getting the type like this might break with extraction???
            entryIndex = _class->addEntry(group->type()->storageType());
        } else {
            // add an entry for the actual value, and one for a pointer to that value
            auto realVal = group->compile();
            auto realIndex = _class->addEntry(realVal);
            entryIndex = _class->addEntry(llvm::PointerType::get(realVal->storageType(), 0));

            auto valPtr = _class->cconstructor()->getEntryPointer(realIndex, "groupval");
            auto ptrPtr = _class->cconstructor()->getEntryPointer(entryIndex, "groupptr");
            _class->constructor()->builder().CreateStore(valPtr, ptrPtr);
        }

        _groupPtrIndexes.emplace(group.get(), entryIndex);
        group->setGetterMethod(_class->entryAccessor(entryIndex));
    }
    auto groupInitializationTime = std::clock() - groupInitializationStart;
    std::cerr << "    Finished group initialization in " << groupInitializationTime << " ms" << std::endl;

    /// NODE WALKING
    auto nodeOrderStart = std::clock();
    // calculate node execution order, using control read/write to make directed graph
    std::vector<Node *> inverseExecutionOrder;
    std::unordered_set<Node *> visitedNodes;
    std::queue<Node *> nodeQueue;

    // step 1: find exposed controls in groups that are written to
    std::set<Node *> writtenNodes;
    addExitNodes(writtenNodes);
    for (const auto &node : writtenNodes) {
        visitedNodes.emplace(node);
        nodeQueue.emplace(node);
    }

    // breadth-first search any connected nodes
    while (!nodeQueue.empty()) {
        auto nextNode = nodeQueue.front();
        nodeQueue.pop();

        inverseExecutionOrder.push_back(nextNode);

        for (const auto &control : *nextNode) {
            for (const auto &connectedControl : control->connections()) {
                // only propagate to the node if it writes to the group - other nodes won't affect output
                // (although they'll still be included in the final list by the check after this one)
                if (!connectedControl->writtenTo() ||
                    visitedNodes.find(connectedControl->node()) != visitedNodes.end())
                    continue;

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
    auto nodeOrderTime = std::clock() - nodeOrderStart;
    std::cerr << "    Finished node order search in " << nodeOrderTime << std::endl;

    // instantiate nodes in their orders - extracted nodes are created multiple times
    // control values are also assigned
    auto nodeInstantiateStart = std::clock();
    for (ssize_t i = inverseExecutionOrder.size() - 1; i >= 0; i--) {
        auto node = inverseExecutionOrder[i];
        auto nodeClass = nodeClasses.find(node)->second;

        // instantiate the node n times
        auto loopSize = node->extracted() ? MaximCodegen::ArrayType::arraySize : 1;

        auto entryIndex = _class->addEntry(llvm::ArrayType::get(nodeClass->storageType(), loopSize));
        node->setGetterMethod(_class->entryAccessor(entryIndex));

        // todo: these two loops really should be refactored

        // CONSTRUCTOR LOOP
        {
            auto &b = _class->constructor()->builder();
            auto entryArrPtr = _class->cconstructor()->getEntryPointer(entryIndex, "entryarr");
            auto indexPtr = _class->constructor()->allocaBuilder().CreateAlloca(llvm::Type::getInt8Ty(runtime()->ctx()->llvm()), nullptr, "index");
            b.CreateStore(runtime()->ctx()->constInt(8, 0, false), indexPtr);

            auto loopCheckBlock = llvm::BasicBlock::Create(runtime()->ctx()->llvm(), "loopcheck", _class->constructor()->get(module()));
            auto loopRunBlock = llvm::BasicBlock::Create(runtime()->ctx()->llvm(), "looprun", _class->constructor()->get(module()));
            auto loopEndBlock = llvm::BasicBlock::Create(runtime()->ctx()->llvm(), "loopend", _class->constructor()->get(module()));

            b.CreateBr(loopCheckBlock);
            b.SetInsertPoint(loopCheckBlock);

            auto currentIndex = b.CreateLoad(indexPtr, "currentindex");
            auto cond = b.CreateICmpULT(currentIndex, runtime()->ctx()->constInt(8, loopSize, false), "loopcond");
            b.CreateCondBr(cond, loopRunBlock, loopEndBlock);

            b.SetInsertPoint(loopRunBlock);
            auto entryPtr = b.CreateGEP(entryArrPtr, {
                runtime()->ctx()->constInt(64, 0, false),
                currentIndex
            }, "entry.ptr");

            for (const auto &control : *node) {
                auto instId = control->instanceId();
                if (instId < 0) continue;

                auto controlPtr = nodeClass->getEntryPointer(_class->constructor()->builder(), instId,
                                                             entryPtr, "controlinst");

                auto groupPtrIndex = _groupPtrIndexes.find(control->group());
                assert(groupPtrIndex != _groupPtrIndexes.end());

                auto groupPtrPtr = _class->cconstructor()->getEntryPointer(groupPtrIndex->second, "groupptrptr");
                llvm::Value *groupPtr = b.CreateLoad(groupPtrPtr, "groupptr");
                auto groupIndexPtr = groupPtr;

                // get the pointer for the specific index we're at if this is control is in an extracted group
                // AND it's not an extractor control (since these get access to the entire array)
                if (control->group()->extracted() && !((uint32_t)control->type()->type() & (uint32_t)MaximCommon::ControlType::EXTRACT)) {
                    // each item in the array is a {bool, value}, we only want the value
                    groupIndexPtr = _class->constructor()->builder().CreateGEP(
                        groupPtr,
                        {
                            runtime()->ctx()->constInt(64, 0, false),
                            currentIndex,
                        }
                    );
                }

                _class->constructor()->builder().CreateStore(groupIndexPtr, controlPtr);
            }

            nodeClass->constructor()->call(b, {}, entryPtr, module(), "");

            auto incrIndex = b.CreateAdd(
                currentIndex,
                runtime()->ctx()->constInt(8, 1, false),
                "incr"
            );
            b.CreateStore(incrIndex, indexPtr);
            b.CreateBr(loopCheckBlock);

            b.SetInsertPoint(loopEndBlock);
        }

        // GENERATE LOOP
        {
            auto &b = _class->generate()->builder();
            auto entryArrPtr = _class->generate()->getEntryPointer(entryIndex, "entryarr");
            auto indexPtr = _class->generate()->allocaBuilder().CreateAlloca(llvm::Type::getInt8Ty(runtime()->ctx()->llvm()), nullptr, "index");
            b.CreateStore(runtime()->ctx()->constInt(8, 0, false), indexPtr);

            auto loopCheckBlock = llvm::BasicBlock::Create(runtime()->ctx()->llvm(), "loopcheck", _class->generate()->get(module()));
            auto loopRunBlock = llvm::BasicBlock::Create(runtime()->ctx()->llvm(), "looprun", _class->generate()->get(module()));
            auto generateBlock = llvm::BasicBlock::Create(runtime()->ctx()->llvm(), "generate", _class->generate()->get(module()));
            auto generateEndBlock = llvm::BasicBlock::Create(runtime()->ctx()->llvm(), "generateend", _class->generate()->get(module()));
            auto loopEndBlock = llvm::BasicBlock::Create(runtime()->ctx()->llvm(), "loopend", _class->generate()->get(module()));

            b.CreateBr(loopCheckBlock);
            b.SetInsertPoint(loopCheckBlock);

            auto currentIndex = b.CreateLoad(indexPtr, "currentindex");
            auto cond = b.CreateICmpULT(currentIndex, runtime()->ctx()->constInt(8, loopSize, false), "loopcond");
            b.CreateCondBr(cond, loopRunBlock, loopEndBlock);

            b.SetInsertPoint(loopRunBlock);
            auto entryPtr = b.CreateGEP(entryArrPtr, {
                runtime()->ctx()->constInt(64, 0, false),
                currentIndex
            }, "entry.ptr");

            // Current solution to reducing CPU usage for voices:
            // Nodes that are extracted are only 'executed' if they don't have any inputs (controls read from),
            // or one of their inputs has its 'active' flag set.
            // TODO:
            // This is kinda hacky and can be counter-intuitive in some cases. It would be better to have each item
            // in an array have an 'active' flag, and an extraction field (ie nodes surrounded by extractors) is only
            // active if any of the inputs are. The key here is that the _entire field_ is either active or not,
            // not just individual nodes (which can lead to some counterintuitive results when an active node is fed
            // with an inactive input in an intermediate state).
            // But graph traversal is hard, so I'm leaving that to my future self.

            llvm::Value *shouldGenerate = nullptr;
            if (loopSize > 1) {
                std::vector<llvm::Value *> generateBools;
                for (const auto &control : *node) {
                    auto instId = control->instanceId();
                    if (!control->readFrom() || instId < 0) continue;

                    auto controlPtr = nodeClass->getEntryPointer(b, instId, entryPtr, "controlinst");

                    // array types are always active (because they don't store an active flag...)
                    llvm::Value *controlActive = llvm::ConstantInt::get(llvm::Type::getInt1Ty(runtime()->ctx()->llvm()), 1, false);
                    auto loadedPtr = b.CreateLoad(controlPtr);
                    if (loadedPtr->getType()->getPointerElementType()->isStructTy()) {
                        controlActive = b.CreateLoad(b.CreateGEP(
                            loadedPtr,
                            {
                                runtime()->ctx()->constInt(64, 0, false),
                                runtime()->ctx()->constInt(32, 0, false)
                            }
                        ), "inputactive");
                    }
                    generateBools.push_back(controlActive);
                }

                if (!generateBools.empty()) {
                    shouldGenerate = generateBools[0];
                    for (size_t generateI = 1; generateI < generateBools.size(); generateI++) {
                        shouldGenerate = b.CreateOr(shouldGenerate, generateBools[generateI], "shouldgenerate");
                    }
                }
            }

            if (shouldGenerate) {
                b.CreateCondBr(shouldGenerate, generateBlock, generateEndBlock);
            } else {
                b.CreateBr(generateBlock);
            }

            b.SetInsertPoint(generateBlock);
            nodeClass->generate()->call(b, {}, entryPtr, module(), "");
            b.CreateBr(generateEndBlock);
            b.SetInsertPoint(generateEndBlock);

            auto incrIndex = b.CreateAdd(
                currentIndex,
                runtime()->ctx()->constInt(8, 1, false),
                "incr"
            );
            b.CreateStore(incrIndex, indexPtr);
            b.CreateBr(loopCheckBlock);

            b.SetInsertPoint(loopEndBlock);
        }
    }
    auto nodeInstantiateTime = std::clock() - nodeInstantiateStart;
    std::cerr << "    Finished node instantiation in " << nodeInstantiateTime << " ms" << std::endl;

    _class->complete();
    deploy();

    return _class.get();
}

void Surface::addNode(Node *node) {
    _nodes.emplace(node);
    scheduleGraphUpdate();
}

void Surface::removeNode(Node *node) {
    _nodes.erase(node);
    scheduleGraphUpdate();
}

void Surface::pullGetterMethod(MaximCodegen::ComposableModuleClassMethod *method) {
    RuntimeUnit::pullGetterMethod(method);

    for (const auto &group : _controlGroups) {
        group->pullGetterMethod();
    }

    for (const auto &node : _nodes) {
        node->pullGetterMethod();
    }
}

void *Surface::updateCurrentPtr(void *parentCtx) {
    auto selfPtr = RuntimeUnit::updateCurrentPtr(parentCtx);

    for (const auto &group : _controlGroups) {
        group->updateCurrentPtr(selfPtr);
    }

    for (const auto &node : _nodes) {
        node->updateCurrentPtr(selfPtr);
    }

    return selfPtr;
}

void Surface::addExitNodes(std::set<MaximRuntime::Node *> &queue) {
    for (const auto &node : _nodes) {
        auto addNode = false;
        for (const std::unique_ptr<Control> &control : *node) {
            if (!control->exposer() || !control->writtenTo()) continue;

            addNode = true;
            break;
        }

        if (addNode) queue.emplace(node);
    }
}

void Surface::saveValue() {
    for (const auto &node : _nodes) {
        node->saveValue();
    }
}

void Surface::restoreValue() {
    std::cout << "Restoring node values" << std::endl;
    for (const auto &node : _nodes) {
        node->restoreValue();
    }
    std::cout << "Finished restoring values" << std::endl;
}
