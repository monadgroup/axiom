#pragma once

#include "Surface.h"
#include "IONode.h"

namespace MaximRuntime {

    class RootSurface : public Surface {
    public:
        AxiomCommon::Event<size_t> automationCountChanged;
        AxiomCommon::Event<size_t> automationFiddled;

        explicit RootSurface(Runtime *runtime);

        void *getValuePtr(void *parentCtx) override;

        void addExitNodes(std::set<Node *> &queue) override;

        size_t getAutomationCount() const { return _currentAutomationIndex; }

        IONode *getAutomationNode(size_t index) const;

        IONode *addAutomationNode();

        void nodeFiddled(IONode *node);

        void removeNode(Node *node) override;

        IONode *input;
        IONode *output;

    private:
        size_t _currentAutomationIndex = 0;
        std::unordered_map<size_t, IONode*> _automationNodes;
        std::unordered_map<IONode*, size_t> _reverseAutomationNodes;
    };

}
