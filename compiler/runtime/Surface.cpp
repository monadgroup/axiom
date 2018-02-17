#include "Surface.h"

#include <unordered_set>
#include <queue>
#include <llvm/Transforms/Utils/Cloning.h>

#include "../codegen/MaximContext.h"
#include "Node.h"
#include "Control.h"

using namespace MaximRuntime;

Surface::Surface(Runtime *runtime)
    : _runtime(runtime), _module("surface", runtime->context.llvm()), _instFunc(&runtime->context, &_module) {
    _module.setDataLayout(runtime->context.dataLayout());
}

Surface::~Surface() {
    if (_hasHandle) {
        _runtime->removeModule(_handle);
    }
}

void Surface::addNode(Node *node) {
    _nodes.push_back(node);
    markAsDirty();
}

void Surface::removeNode(Node *node) {
    auto index = std::find(_nodes.begin(), _nodes.end(), node);
    if (index != _nodes.end()) _nodes.erase(index);
}

void Surface::markAsDirty() {
    _isDirty = true;
    _instFunc.reset();
    if (parent) parent->markAsDirty();
}

MaximCodegen::InstantiableFunction *Surface::getFunction() {
    if (!_isDirty) {
        return &_instFunc;
    }
    _isDirty = false;

    // get order of nodes with breadth-first search
    // search seed is the exposed writer nodes
    std::unordered_set<Node*> visitedNodes;
    std::queue<Node*> visitQueue;
    std::vector<Node*> inverseOrder;

    for (const auto &group : _groups) {
        // skip if group isn't exposed
        if (!group->isExposed()) continue;

        // skip if group has no writer (ie is an input group)
        if (!group->writer()) continue;

        auto writerNode = group->writer()->node();
        if (visitedNodes.find(writerNode) != visitedNodes.end()) continue;

        visitedNodes.emplace(writerNode);
        visitQueue.push(writerNode);
    }

    while (!visitQueue.empty()) {
        auto visitNode = visitQueue.front();
        visitQueue.pop();

        inverseOrder.push_back(visitNode);

        for (const auto &control : visitNode->controls()) {
            for (const auto &connection : control->connections()) {
                auto connectedNode = connection->node();
                if (visitedNodes.find(connectedNode) != visitedNodes.end()) continue;
                visitedNodes.emplace(connectedNode);
                visitQueue.push(connectedNode);
            }
        }
    }

    // add nodes that haven't been included yet
    for (const auto &node : _nodes) {
        if (visitedNodes.find(node) != visitedNodes.end()) continue;

        inverseOrder.push_back(node);
    }

    // add nodes and calls to them in the functions, assign controls to their globals
    if (!inverseOrder.empty()) {
        for (size_t i = inverseOrder.size() - 1; i >= 0; i--) {
            auto node = inverseOrder[i];
            auto nodeFunc = node->getFunction();
            auto ptr = _instFunc.addInstantiable(nodeFunc);

            // used when the control is initialized
            for (const auto &control : node->controls()) {
                control->control()->storagePointer = control->group()->global();
            }
            node->getFunction()->complete();

            MaximCodegen::CreateCall(_instFunc.builder(), nodeFunc->generateFunc(&_module), {ptr}, "");
        }
    }

    _instFunc.complete();

    if (_hasHandle) _runtime->removeModule(_handle);
    _handle = _runtime->addModule(llvm::CloneModule(_module));
    _hasHandle = true;

    return &_instFunc;
}
