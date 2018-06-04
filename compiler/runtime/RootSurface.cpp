#include "RootSurface.h"

using namespace MaximRuntime;

RootSurface::RootSurface(Runtime *runtime) : Surface(runtime, 0) {
    auto inputNode = std::make_unique<IONode>(this, MaximCommon::ControlType::MIDI, false, true);
    input = inputNode.get();
    addNode(std::move(inputNode));

    auto outputNode = std::make_unique<IONode>(this, MaximCommon::ControlType::NUMBER, true, false);
    output = outputNode.get();
    addNode(std::move(outputNode));
}

void *RootSurface::getValuePtr(void *parentCtx) {
    return parentCtx;
}

void RootSurface::addExitNodes(std::set<MaximRuntime::Node *> &queue) {
    queue.emplace(output);
}

IONode* RootSurface::addAutomationNode() {
    auto newIndex = _currentAutomationIndex++;
    automationCountChanged.trigger(_currentAutomationIndex);
    auto newNode = std::make_unique<IONode>(this, MaximCommon::ControlType::NUMBER, false, true);
    auto ptr = newNode.get();
    addNode(std::move(newNode));
    _automationNodes.emplace(newIndex, ptr);
    _reverseAutomationNodes.emplace(ptr, newIndex);
    return ptr;
}

IONode* RootSurface::getAutomationNode(size_t index) const {
    auto iter = _automationNodes.find(index);
    if (iter == _automationNodes.end()) return nullptr;
    return iter->second;
}

void RootSurface::nodeFiddled(MaximRuntime::IONode *node) {
    auto index = _reverseAutomationNodes.find(node);
    if (index == _reverseAutomationNodes.end()) return;
    automationFiddled.trigger(index->second);
}

void RootSurface::removeNode(MaximRuntime::Node *node) {
    // if we can find the ptr in the map, we know it's an IONode - so no need to use dynamic_cast
    auto index = _reverseAutomationNodes.find(static_cast<MaximRuntime::IONode*>(node));
    if (index != _reverseAutomationNodes.end()) {
        auto num = index->second;
        _automationNodes.erase(num);
        _reverseAutomationNodes.erase(index);

        // reduce the automation index until there's something occupying
        do {
            _currentAutomationIndex--;
        } while (_currentAutomationIndex > 0 && _automationNodes.find(_currentAutomationIndex - 1) == _automationNodes.end());
        automationCountChanged.trigger(_currentAutomationIndex);
    }

    Surface::removeNode(node);
}
