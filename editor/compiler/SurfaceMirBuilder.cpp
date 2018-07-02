#include "SurfaceMirBuilder.h"

#include <vector>

#include "../model/objects/Node.h"
#include "../model/objects/ControlSurface.h"
#include "../model/objects/Control.h"
#include "../model/PoolOperators.h"
#include "../model/ModelRoot.h"

using namespace MaximCompiler;

struct ValueGroup {
    std::vector<QUuid> controls;

    void mergeInto(ValueGroup *target) {
        target->controls.insert(target->controls.end(), controls.begin(), controls.end());
    }
};

void SurfaceMirBuilder::build(Runtime &runtime, MaximCompiler::Transaction &transaction, AxiomModel::NodeSurface *surface) {
    auto mir = transaction.buildSurface(surface->getRuntimeId(runtime), surface->name().toStdString());

    // build control groups
    std::unordered_map<ValueGroup *, std::unique_ptr<ValueGroup>> groups;
    QHash<QUuid, ValueGroup *> controlGroups;

    for (const auto &node : surface->nodes()) {
        auto controlsContainer = *node->controls().value();
        for (const auto &control : controlsContainer->controls()) {
            if (control->connectedControls().empty()) {
                // assign the control a single group
                auto newGroup = std::make_unique<ValueGroup>();
                newGroup->controls.push_back(control->uuid());
                controlGroups.insert(control->uuid(), newGroup.get());
                groups.emplace(newGroup.get(), std::move(newGroup));
            } else {
                auto myGroupIndex = controlGroups.find(control->uuid());
                ValueGroup *myGroup;
                if (myGroupIndex == controlGroups.end()) {
                    auto newGroup = std::make_unique<ValueGroup>();
                    newGroup->controls.push_back(control->uuid());
                    myGroup = newGroup.get();
                    controlGroups.insert(control->uuid(), newGroup.get());
                    groups.emplace(newGroup.get(), std::move(newGroup));
                } else {
                    myGroup = myGroupIndex.value();
                }

                for (const auto &connectedControl : control->connectedControls()) {
                    auto connectedGroupIndex = controlGroups.find(connectedControl);
                    if (connectedGroupIndex == controlGroups.end()) {
                        // add the control to our group
                        myGroup->controls.push_back(connectedControl);
                        controlGroups.insert(connectedControl, myGroup);
                    } else if (connectedGroupIndex.value() != myGroup) {
                        auto connectedGroup = connectedGroupIndex.value();
                        // merge the group into ours: first, update all entries in controlGroups
                        for (const auto &groupControl : connectedGroup->controls) {
                            controlGroups.insert(groupControl, myGroup);
                        }

                        // next merge the controls list in the group
                        connectedGroup->mergeInto(myGroup);

                        // now remove it from the groups array
                        groups.erase(connectedGroup);
                    };
                }
            }
        }
    }

    // build a map of value groups to indices while building the MIR
    std::unordered_map<ValueGroup *, size_t> valueGroupIndices;
    std::vector<size_t> portals;
    size_t index = 0;
    for (const auto &pair : groups) {
        valueGroupIndices.emplace(pair.first, index++);

        // todo
    }

    // build nodes
}
