#include "SurfaceMirBuilder.h"

#include "../model/objects/NodeSurface.h"
#include "../model/objects/Node.h"
#include "../model/objects/ControlSurface.h"
#include "../model/objects/NumControl.h"
#include "../model/PoolOperators.h"
#include "../model/ModelRoot.h"

using namespace MaximCompiler;

struct ValueGroup {
    std::vector<QUuid> controls;

    void mergeInto(ValueGroup *target) {
        target->controls.insert(target->controls.end(), controls.begin(), controls.end());
    }
};

bool areAnyExposed(const std::vector<AxiomModel::Control *> &controls) {
    for (const auto &control : controls) {
        if (!control->exposerUuid().isNull()) {
            return true;
        }
    }
    return false;
}

AxiomModel::Control::ControlType getGroupType(const std::vector<AxiomModel::Control *> &controls) {
    assert(!controls.empty());
    for (const auto &control : controls) {
        if (control->controlType() == AxiomModel::Control::ControlType::NUM_EXTRACT ||
            control->controlType() == AxiomModel::Control::ControlType::MIDI_EXTRACT) {
            return control->controlType();
        }
    }
    return controls[0]->controlType();
}

void
SurfaceMirBuilder::build(Runtime &runtime, MaximCompiler::Transaction &transaction, AxiomModel::NodeSurface *surface,
                         const QHash<QUuid, NodeMirData> &nodeData) {
    auto mir = transaction.buildSurface(surface->getRuntimeId(runtime), surface->name());

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
    std::vector<size_t> sockets;
    size_t index = 0;
    for (const auto &pair : groups) {
        auto currentIndex = index++;
        valueGroupIndices.emplace(pair.first, currentIndex);
        auto controlPointers = AxiomModel::collect(
            AxiomModel::findMap(pair.first->controls, surface->root()->controls()));

        auto isExposed = areAnyExposed(controlPointers);
        auto groupType = getGroupType(controlPointers);

        auto vartype = VarType::ofControl(fromModelType(groupType));

        if (isExposed) {
            auto socketIndex = sockets.size();
            sockets.push_back(currentIndex);
            mir.addValueGroup(std::move(vartype), ValueGroupSource::socket(socketIndex));
            continue;
        } else if (groupType == AxiomModel::Control::ControlType::NUM_SCALAR) {
            // determine if the group is written to
            auto isWrittenTo = false;
            for (const auto &control : controlPointers) {
                auto controlNodeData = nodeData.find(control->surface()->node()->uuid());
                assert(controlNodeData != nodeData.end());

                auto controlData = controlNodeData->controls.find(control->uuid());
                assert(controlData != controlNodeData->controls.end());

                if (controlData->writtenTo) {
                    isWrittenTo = true;
                    break;
                }
            }

            if (!isWrittenTo) {
                // save a constant value
                auto numControl = dynamic_cast<AxiomModel::NumControl *>(controlPointers[0]);
                assert(numControl);

                auto numVal = numControl->value();
                if (numVal.left != 0 || numVal.right != 0 || (int) numVal.form != 0) {
                    auto constVal = ConstantValue::num(numVal);
                    mir.addValueGroup(std::move(vartype), ValueGroupSource::default_val(std::move(constVal)));
                    continue;
                }
            }
        }

        mir.addValueGroup(std::move(vartype), ValueGroupSource::none());
    }

    // build nodes
    /*for (const auto &node : surface->nodes()) {

    }*/

    // todo
}
